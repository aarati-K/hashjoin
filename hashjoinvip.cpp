#include "hashjoinvip.h"

Hashjoinvip::Hashjoinvip() {
    output = NULL;
    dict = NULL;
    entries = NULL;
    acc_dict = NULL;
    acc_entries = NULL;
    entriesOffset = 0;
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
    cout << "Hashpower: " << hashpower << endl;
    dict = (DictEntry*)malloc(hashmap_size*sizeof(DictEntry));
    entries = (KV*)malloc(max_entries*sizeof(KV));
    acc_dict = (AccessCount**)malloc((hashmap_size+1)*sizeof(AccessCount*));
    acc_entries = (AccessCount*)malloc((max_entries+1)*sizeof(AccessCount));
    if (!dict || !entries || !acc_dict || !acc_entries) {
        cout << "Failed initializing hashmap memory" << endl;
    }
    memset(dict, 0, hashmap_size*sizeof(DictEntry));
    memset(entries, 0, max_entries*sizeof(KV));
    memset(acc_dict, 0, hashmap_size*sizeof(AccessCount*));
    memset(acc_entries, 0, max_entries*sizeof(AccessCount));

    acc_dict[0] = &acc_entries[0];
    acc_entries[0].next = &acc_entries[0];
}

inline void Hashjoinvip::insert(int key, void* ptr) {
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
    entries[entriesOffset].next = dict[hash_loc].head;
    acc_entries[entriesOffset+1].next = acc_dict[hash_loc+1];
    dict[hash_loc].head = &entries[entriesOffset];
    dict[hash_loc].budget += 1;
    acc_dict[hash_loc+1] = &acc_entries[entriesOffset+1];
    entriesOffset += 1;
}

void* Hashjoinvip::exec(Table &fact, int factcol, Table &dim, int dimcol) {
    ColumnInfo f = fact.getColumnInfo(factcol);
    ColumnInfo d = dim.getColumnInfo(dimcol);
    Metrics m;
    struct timespec start_time, end_time;

    // Assuming join is on integer attributes
    initHashmap(d.numtuples);
    output = malloc(long(f.numtuples)*(f.incr + d.incr)); // conservative

    // Build hashmap
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    void *addr = d.startAddr;
    int incr = d.incr;
    for (int i=0; i<d.numtuples; i++) {
        insert(*((int*)addr), addr - d.offset);
        addr += incr;
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    m.build_time = getTimeDiff(start_time, end_time);

    // Probe hashmap
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    int n_learning = (d.numtuples < f.numtuples/60) ? d.numtuples : f.numtuples/60;
    n_learning = f.numtuples;
    addr = f.startAddr;
    incr = f.incr;
    int key, hash_loc;
    uint8_t budget;
    KV *ptr, *min_count_ptr;
    AccessCount *acc_ptr, *min_count_acc_ptr;
    void* output_it = output;
    int i = 0;
    // int num_swaps = 0;
    for (; i<n_learning; i++) {
        key = *((int*)addr);
        hash_loc = (key*prime) >> (32 - hashpower);
        ptr = dict[hash_loc].head;
        budget = !!(dict[hash_loc].budget);
        min_count_ptr = ptr;
        acc_ptr = acc_dict[(hash_loc+1)*budget];
        min_count_acc_ptr = acc_ptr;
        while (ptr != NULL) {
            if (ptr->key == key) {
                // copy to output
                memcpy(output_it, addr - f.offset, f.incr);
                output_it += f.incr;
                memcpy(output_it, ptr->ptr, d.incr);
                output_it += d.incr;
                acc_ptr->count += 1;
                break; // assuming pk-fk join
            }
            Is it possible to write this if statement without using if?
            if (budget && acc_ptr->count < min_count_acc_ptr->count) {
                min_count_acc_ptr = acc_ptr;
                min_count_ptr = ptr;
            }
            m.displacement += 1;
            ptr = ptr->next;
            acc_ptr = acc_ptr->next;
        }
        // Swap
        if (budget && acc_ptr->count > min_count_acc_ptr->count) {
            // num_swaps += 1;

            uint8_t count = acc_ptr->count;
            acc_ptr->count = min_count_acc_ptr->count;
            min_count_acc_ptr->count = count;

            ptr->key = min_count_ptr->key;
            min_count_ptr->key = key;

            void* payload = ptr->ptr;
            ptr->ptr = min_count_ptr->ptr;
            min_count_ptr->ptr = payload;
        }
        addr += incr;
        dict[hash_loc].budget -= budget;
    }
    // for (; i<f.numtuples; i++) {
    //     key = *((int*)addr);
    //     hash_loc = (key*prime) >> (32 - hashpower);
    //     ptr = dict[hash_loc].head;
    //     while (ptr != NULL) {
    //         if (ptr->key == key) {
    //             // copy to output
    //             memcpy(output_it, addr - f.offset, f.incr);
    //             output_it += f.incr;
    //             memcpy(output_it, ptr->ptr, d.incr);
    //             output_it += d.incr;
    //             break; // assuming pk-fk join
    //         }
    //         m.displacement += 1;
    //         ptr = ptr->next;
    //     }
    //     addr += incr;
    // }
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    m.probe_and_materialize_time = getTimeDiff(start_time, end_time);

    m.total_time = m.build_time + m.probe_and_materialize_time;
    
    cout << "Total time: " << m.total_time << endl;
    cout << "Build time: " << m.build_time << endl;
    cout << "Probe + Materialize time: " << m.probe_and_materialize_time << endl;
    cout << "Displacement: " << m.displacement << endl;
    // cout << "Num swaps: " << num_swaps << endl;

    // // Store the hashmap to a file
    // ofstream file("data/dim_learned.tbl");
    // stack<int> s;
    // for (int i=0; i<hashmap_size; i++) {
    //     // Iterate over buckets
    //     ptr = dict[i].head;
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
