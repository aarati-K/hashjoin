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

inline void Hashjoin::insert(int key, void* ptr) {
    // if (!initialized) {
    //     cout << "Hashmap not initialized yet" << endl;
    //     return;
    // }
    // if (entriesOffset == max_entries) {
    //     cout << "Can't insert anymore" << endl;
    //     return;
    // }
    int hash_loc = (key*prime) >> (32 - hashpower);
    entries[entriesOffset].key = key;
    entries[entriesOffset].ptr = ptr;
    entries[entriesOffset].next = dict[hash_loc];
    dict[hash_loc] = &entries[entriesOffset];
    entriesOffset += 1;
}

void* Hashjoin::exec(Table &fact, int factcol, Table &dim, int dimcol) {
    ColumnInfo f = fact.getColumnInfo(factcol);
    ColumnInfo d = dim.getColumnInfo(dimcol);
    struct timespec start_time, end_time;
    long build_time, probe_time;
    int displacement = 0;

    // Assuming join is on integer attributes
    initHashmap(d.numtuples);
    output = malloc(f.numtuples*(f.incr + d.incr)); // conservative

    // Build hashmap
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    void *addr = d.startAddr;
    int incr = d.incr;
    for (int i=0; i<d.numtuples; i++) {
        insert(*((int*)addr), addr - d.offset);
        addr += incr;
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    build_time = getTimeDiff(start_time, end_time);

    // Probe hashmap
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    addr = f.startAddr;
    incr = f.incr;
    int key, hash_loc;
    KV* ptr;
    void* output_it = output;
    for (int i=0; i<f.numtuples; i++) {
        key = *((int*)addr);
        hash_loc = (key*prime) >> (32 - hashpower);
        ptr = dict[hash_loc];
        while (ptr != NULL) {
            if (ptr->key == key) {
                // copy to output
                memcpy(output_it, addr - f.offset, f.incr);
                output_it += f.incr;
                memcpy(output_it, ptr->ptr, d.incr);
                output_it += d.incr;
                break; // assuming pk-fk join
            }
            displacement += 1;
            ptr = ptr->next;
        }
        addr += incr;
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    probe_time = getTimeDiff(start_time, end_time);

    cout << "Build time: " << build_time << endl;
    cout << "Probe time: " << probe_time << endl;
    cout << "Total time: " << build_time + probe_time << endl;
    cout << "Displacement: " << displacement << endl;
    return output;
}
