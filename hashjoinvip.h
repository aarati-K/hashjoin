#include <cstdlib>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
// #include <stack>
// #include <fstream>
#define _INTEL_INTRINSICS_ 1

#if _INTEL_INTRINSICS_
#include <x86intrin.h>
#endif

#include "table.h"
#include "metrics.h"

using namespace std;

#ifndef _KV_
#define _KV_

typedef struct KV {
    ulong key = 0;
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
    void insert (ulong key, void* ptr);
    void *exec(Table &fact, int factcol, Table &dim, int dimcol);
protected:
    ulong _murmurHash(ulong);
    void build();
private:
    void** output;
    KV **dict;
    KV *entries;
    AccessCount *acc_entries;
    int entriesOffset;
    int accessesOffset;
    int max_entries;
    ulong hashmap_size;
    int hashpower;
    uint prime = 472882027;
    bool initialized = false;
    float max_sf = 2;
};

#endif // _HASHJOINVIP_H_
