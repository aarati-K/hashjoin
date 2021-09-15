#include <cstdlib>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

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

#ifndef _HASHJOIN_H_
#define _HASHJOIN_H_

class Hashjoin {
public:
    Hashjoin();
    void initHashmap (int n);
    void insert (ulong key, void* ptr);
    void *exec(Table &fact, int factcol, Table &dim, int dimcol);
protected:
    ulong _murmurHash(ulong);
    void build();
private:
    void* output;
    KV **dict;
    KV *entries;
    int entriesOffset;
    int max_entries;
    ulong hashmap_size;
    int hashpower;
    uint prime = 472882027;
    bool initialized = false;
    float max_sf = 1;
};

#endif // _HASHJOIN_H_
