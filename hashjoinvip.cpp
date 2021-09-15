#include "hashjoinvip.h"

Hashjoinvip::Hashjoinvip() {
    output = NULL;
    dict = NULL;
    entries = NULL;
    // acc_dict = NULL;
    acc_entries = NULL;
    // budget_per_bucket = NULL;
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
    // acc_dict = (int*)malloc((hashmap_size+1)*sizeof(int));
    acc_entries = (AccessCount*)malloc((2*max_entries+1)*sizeof(AccessCount));
    // budget_per_bucket = (AccessCount*)malloc((hashmap_size*sizeof(AccessCount));
    if (!dict || !entries || !acc_entries) {
        cout << "Failed initializing hashmap memory" << endl;
    }
    memset(dict, 0, hashmap_size*sizeof(KV*));
    memset(entries, 0, max_entries*sizeof(KV));
    // memset(acc_dict, 0, (hashmap_size+1)*sizeof(AccessCount*));
    memset(acc_entries, 0, (2*max_entries+1)*sizeof(AccessCount));
    // memset(budget_per_bucket, 0, hashmap_size*sizeof(uint8_t));

    // That should happen implicitly with memset
    // acc_dict[0] = 0;
    // acc_entries[0].next = 0;
    accessesOffset = hashmap_size + 1;
}

inline void Hashjoinvip::insert(ulong key, void* ptr) {
    // if (!initialized) {
    //     cout << "Hashmap not initialized yet" << endl;
    //     return;
    // }
    // if (entriesOffset == max_entries) {
    //     cout << "Can't insert anymore" << endl;
    //     return;
    // }
    ulong hash_loc = _murmurHash(key);
    entries[entriesOffset].key = key;
    entries[entriesOffset].ptr = ptr;
    entries[entriesOffset].next = dict[hash_loc];
    dict[hash_loc] = &entries[entriesOffset];
    acc_entries[hash_loc+1].count[0] += 1; // budget
    entriesOffset += 1;
}

void* Hashjoinvip::exec(Table &fact, int factcol, Table &dim, int dimcol) {
    ColumnInfo f = fact.getColumnInfo(factcol);
    ColumnInfo d = dim.getColumnInfo(dimcol);
    Metrics m;
    ulong cycles;

    // Assuming join is on integer attributes
    initHashmap(d.numtuples);
    output = malloc(ulong(f.numtuples)*(f.incr + d.incr)); // conservative

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
    uint8_t budget;
    uint8_t flag;
    KV *ptr, *min_count_ptr;
    int acc_offset, prev_acc_offset, min_count_acc_offset;
    int acc_index, min_count_acc_index;
    void* output_it = output;
    int i = 0;
    // int num_swaps = 0;
    for (; i<n_learning; i++) {
        key = *((ulong*)addr);
        hash_loc = _murmurHash(key);
        ptr = dict[hash_loc];
        budget = !!(acc_entries[hash_loc+1].count[0]);
        acc_entries[hash_loc+1].count[0] -= budget;
        min_count_ptr = ptr;
        acc_offset = (hash_loc+1)*budget;
        // if (budget && !acc_offset) {
        //     acc_offset = accessesOffset;
        //     acc_dict[(hash_loc+1)] = accessesOffset;
        //     accessesOffset += 1;
        // }
        acc_index = 1;
        prev_acc_offset = 0;
        min_count_acc_offset = acc_offset;
        min_count_acc_index = 1;
        while (ptr != NULL) {
            if (ptr->key == key) {
                // copy to output
                memcpy(output_it, (char*)addr - f.offset, f.incr);
                output_it = (char*)output_it + f.incr;
                memcpy(output_it, ptr->ptr, d.incr);
                output_it = (char*)output_it + d.incr;
                acc_entries[acc_offset].count[acc_index] += 1;
                break; // assuming pk-fk join
            }
            if (budget && acc_entries[acc_offset].count[acc_index] < acc_entries[min_count_acc_offset].count[min_count_acc_index]) {
                min_count_acc_offset = acc_offset;
                min_count_acc_index = acc_index;
                min_count_ptr = ptr;
            }
            // flag = 1 & (acc_entries[acc_offset].count - acc_entries[min_count_acc_offset].count) >> 7;
            // min_count_acc_offset = min_count_acc_offset + flag*(acc_offset - min_count_acc_offset);
            // min_count_ptr = min_count_ptr + flag*(ptr - min_count_ptr);
            m.displacement += 1;
            ptr = ptr->next;
            acc_index = (acc_index + 1) % 4;
            if (!acc_index) {
                prev_acc_offset = acc_offset;
                acc_offset = acc_entries[acc_offset].next;
                if (prev_acc_offset && !acc_offset) {
                    acc_offset = accessesOffset;
                    acc_entries[prev_acc_offset].next = acc_offset;
                    accessesOffset += 1;
                }
            }
        }
        // Swap
        if (budget && acc_entries[acc_offset].count[acc_index] > acc_entries[min_count_acc_offset].count[min_count_acc_index]) {
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
    for (ulong addr=0; addr < (2*max_entries+1)*sizeof(AccessCount); addr += 64) {
        _mm_clflushopt((char*)acc_entries+addr);
    }
    m.learn_cycles = rdpmc_core_cycles() - cycles;

    cycles = rdpmc_core_cycles();
    for (; i<f.numtuples; i++) {
        key = *((ulong*)addr);
        hash_loc = _murmurHash(key);
        ptr = dict[hash_loc];
        while (ptr != NULL) {
            if (ptr->key == key) {
                // copy to output
                memcpy(output_it, (char*)addr - f.offset, f.incr);
                output_it = (char*)output_it + f.incr;
                memcpy(output_it, ptr->ptr, d.incr);
                output_it = (char*)output_it + d.incr;
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
