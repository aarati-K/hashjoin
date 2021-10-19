#include "hashjoinvip.h"

Hashjoinvip::Hashjoinvip() {
    output = NULL;
    dict = NULL;
    entries = NULL;
    acc_entries = NULL;
    entriesOffset = 0;
    accessesOffset = 0;
    hashmap_size = 0;
    max_entries = 0;
}

void Hashjoinvip::initHashmap(int n) {
    max_entries = n;
    hashmap_size = 1;
    hashpower = 0;
    while (max_sf*hashmap_size < max_entries) {
        hashmap_size = 2*hashmap_size;
        hashpower += 1;
    }
    // cout << "Hashpower: " << hashpower << endl;
    dict = (KV**)malloc(hashmap_size*sizeof(KV*));
    entries = (KV*)malloc(max_entries*sizeof(KV));
    acc_entries = (AccessCount*)malloc((2*max_entries)*sizeof(AccessCount));
    if (!dict || !entries || !acc_entries) {
        cout << "Failed initializing hashmap memory" << endl;
    }
    memset(dict, 0, hashmap_size*sizeof(KV*));
    memset(entries, 0, max_entries*sizeof(KV));
    memset(acc_entries, 0, (max_entries+1)*sizeof(AccessCount));

    accessesOffset = hashmap_size;
}

inline void Hashjoinvip::insert(ulong key, void* ptr) {
    ulong hash_loc = _murmurHash(key);
    entries[entriesOffset].key = key;
    entries[entriesOffset].ptr = ptr;
    entries[entriesOffset].next = dict[hash_loc];
    dict[hash_loc] = &entries[entriesOffset];
    entriesOffset += 1;
}

void* Hashjoinvip::exec(Table &fact, int factcol, Table &dim, int dimcol) {
    ColumnInfo f = fact.getColumnInfo(factcol);
    ColumnInfo d = dim.getColumnInfo(dimcol);
    Metrics m;
    ulong cycles;

    // Assuming join is on integer attributes
    initHashmap(d.numtuples);
    output = (void**)malloc(ulong(f.numtuples)*2*sizeof(void*)); // conservative

    // Build hashmap
    getMetricsStart(m);
    cycles = rdpmc_core_cycles();
    void *addr = d.startAddr;
    ulong incr = d.incr;
    for (int i=0; i<d.numtuples; i++) {
        insert(*((ulong*)addr), (char*)addr - d.offset);
        addr = (char*)addr + incr;
    }
    m.build_cycles = rdpmc_core_cycles() - cycles;

    // Probe hashmap
    cycles = rdpmc_core_cycles();
    int n_learning = (d.numtuples < f.numtuples/60) ? d.numtuples : f.numtuples/60;
    addr = f.startAddr;
    incr = f.incr;
    ulong key, hash_loc;
    // uint8_t budget;
    uint8_t flag;
    KV *ptr, *min_count_ptr;
    int acc_offset, prev_acc_offset, min_count_acc_offset;
    int acc_index, min_count_acc_index;
    void** output_it = output;
    int i = 0;
    // int num_swaps = 0;
    for (; i<n_learning; i++) {
        key = *((ulong*)addr);
        hash_loc = _murmurHash(key);
        ptr = dict[hash_loc];
        min_count_ptr = ptr;
        acc_offset = hash_loc;
        acc_index = 0;
        prev_acc_offset = hash_loc;
        min_count_acc_offset = acc_offset;
        min_count_acc_index = 0;
        while (ptr != NULL) {
            if (ptr->key == key) {
                // copy to output
                *(output_it) = (char*)addr - f.offset;
                output_it = output_it + 1;
                *(output_it) = ptr->ptr;
                output_it = output_it + 1;
                acc_entries[acc_offset].count[acc_index] += 1;
                break; // assuming pk-fk join
            }
            if (acc_entries[acc_offset].count[acc_index] < acc_entries[min_count_acc_offset].count[min_count_acc_index]) {
                min_count_acc_offset = acc_offset;
                min_count_acc_index = acc_index;
                min_count_ptr = ptr;
            }
            m.displacement += 1;
            ptr = ptr->next;
            acc_index = (acc_index + 1) % 4;
            if (!acc_index) {
                prev_acc_offset = acc_offset;
                acc_offset = acc_entries[acc_offset].next;
                if (!acc_offset) {
                    acc_offset = accessesOffset;
                    acc_entries[prev_acc_offset].next = acc_offset;
                    accessesOffset += 1;
                }
            }
        }
        // Swap
        if (acc_entries[acc_offset].count[acc_index] > acc_entries[min_count_acc_offset].count[min_count_acc_index]) {
            // num_swaps += 1;

            uint8_t count = acc_entries[acc_offset].count[acc_index];
            acc_entries[acc_offset].count[acc_index] = acc_entries[min_count_acc_offset].count[min_count_acc_index];
            acc_entries[min_count_acc_offset].count[min_count_acc_index] = count;

            ptr->key = min_count_ptr->key;
            min_count_ptr->key = key;

            void* payload = ptr->ptr;
            ptr->ptr = min_count_ptr->ptr;
            min_count_ptr->ptr = payload;
        }
        addr = (char*)addr + incr;
    }
    #if _INTEL_INTRINSICS_
    for (ulong addr=0; addr < (2*max_entries+1)*sizeof(AccessCount); addr += 64) {
        _mm_clflushopt((char*)acc_entries+addr);
    }
    #endif
    m.learn_cycles = rdpmc_core_cycles() - cycles;

    cycles = rdpmc_core_cycles();
    for (; i<f.numtuples; i++) {
        key = *((ulong*)addr);
        hash_loc = _murmurHash(key);
        ptr = dict[hash_loc];
        while (ptr != NULL) {
            if (ptr->key == key) {
                // copy to output
                *(output_it) = (char*)addr - f.offset;
                output_it = output_it + 1;
                *(output_it) = ptr->ptr;
                output_it = output_it + 1;
                break; // assuming pk-fk join
            }
            m.displacement += 1;
            ptr = ptr->next;
        }
        addr = (char*)addr + incr;
    }
    m.probe_and_materialize_cycles = rdpmc_core_cycles() - cycles;
    getMetricsEnd(m);
    printMetrics(m);
    
    // cout << "Num swaps: " << num_swaps << endl;

    // // Store the hashmap to a file
    // ofstream file("data/dim_learned.tbl");
    // stack<int> s;
    // for (int i=0; i<hashmap_size; i++) {
    //     // Iterate over buckets
    //     ptr = dict[i];
    //     while (ptr != NULL) {
    //         s.push(ptr->key);
    //         ptr = ptr->next;
    //     }
    //     while (!s.empty()) {
    //         file << s.top() << '|' << s.top() << endl;
    //         s.pop();
    //     }
    // }
    // file.close();
    return output;
}

inline ulong Hashjoinvip::_murmurHash(ulong h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    h = h >> (64 - hashpower);
    return h;
}
