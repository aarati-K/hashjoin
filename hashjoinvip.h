#include <cstdlib>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
// #include <stack>
// #include <fstream>
#include <x86intrin.h>

#include "table.h"
#include "metrics.h"

using namespace std;

#ifndef _KV_
#define _KV_

typedef struct KV {
    int key = 0;
    void* ptr = NULL;       // ptr to data
    struct KV* next = NULL;
} KV;

#endif // _KV_

#ifndef _HASHJOINVIP_H_
#define _HASHJOINVIP_H_

typedef struct AccessCount {
    uint8_t count[4];
    int next = 0;   // index into the acc_entries array
} AccessCount;

class Hashjoinvip {
public:
    Hashjoinvip();
    void initHashmap (int n);
    void insert (int key, void* ptr);
    void *exec(Table &fact, int factcol, Table &dim, int dimcol);
protected:
    void build();
private:
    void* output;
    KV **dict;
    KV *entries;
    // int* acc_dict;
    AccessCount *acc_entries;
    // AccessCount *budget_per_bucket;
    int entriesOffset;
    int accessesOffset;
    int max_entries;
    long hashmap_size;
    int hashpower;
    uint prime = 472882027;
    bool initialized = false;
    float max_sf = 1;
};

#endif // _HASHJOINVIP_H_
