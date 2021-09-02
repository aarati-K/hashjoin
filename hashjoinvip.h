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
    int key = 0;
    void* ptr = NULL;       // ptr to data
    struct KV* next = NULL;
} KV;

#endif // _KV_

#ifndef _HASHJOINVIP_H_
#define _HASHJOINVIP_H_

typedef struct DictEntry {
    KV* head = NULL;
    uint8_t budget = 0;
} DictEntry;

typedef struct AccessCount {
    uint8_t count = 0;
    struct AccessCount *next = NULL;
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
    DictEntry *dict;
    KV *entries;
    AccessCount **acc_dict;
    AccessCount *acc_entries;
    int entriesOffset;
    int max_entries;
    int hashmap_size;
    int hashpower;
    uint prime = 472882027;
    bool initialized = false;
    float max_sf = 2;
};

#endif // _HASHJOINVIP_H_
