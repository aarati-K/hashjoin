#include "hashjoin.h"

Hashjoin::Hashjoin() {
    output = NULL;
    dict = NULL;
    entries = NULL;
    entriesOffset = 0;
    hashmap_size = 0;
    max_entries = 0;
}

void Hashjoin::initHashmap(int n) {
    max_entries = n;
    hashmap_size = 1;
    hashpower = 0;
    while (max_sf*hashmap_size < max_entries) {
        hashmap_size = 2*hashmap_size;
        hashpower += 1;
    }
    cout << "hashpower: " << hashpower << endl;
    dict = (KV**)malloc(hashmap_size*sizeof(KV*));
    entries = (KV*)malloc(max_entries*sizeof(KV));
    if (!dict || !entries) {
        cout << "Failed initializing hashmap memory" << endl;
    }
    memset(dict, 0, hashmap_size*sizeof(KV*));
    memset(entries, 0, max_entries*sizeof(KV));
}

inline void Hashjoin::insert(ulong key, void* ptr) {
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
    entriesOffset += 1;
}

void* Hashjoin::exec(Table &fact, int factcol, Table &dim, int dimcol) {
    ColumnInfo f = fact.getColumnInfo(factcol);
    ColumnInfo d = dim.getColumnInfo(dimcol);
    Metrics m;
    ulong cycles;
    struct timespec start_time, end_time;

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
    addr = f.startAddr;
    incr = f.incr;
    ulong key, hash_loc;
    KV* ptr;
    void* output_it = output;
    for (int i=0; i<f.numtuples; i++) {
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
    return output;
}

inline ulong Hashjoin::_murmurHash(ulong h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    h = h >> (64 - hashpower);
    return h;
}
