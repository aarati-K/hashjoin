#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

using namespace std;

#ifndef _METRICS_H_
#define _METRICS_H_

typedef struct Metrics {
    ulong clockCycles;
    ulong l3Miss;
} Metrics;

unsigned long getTimeDiff(struct timespec, struct  timespec);
unsigned long rdpmc_core_cycles();
unsigned long rdpmc_llc_cache_misses();

// void getMetricsStart(Metrics&);
// void getMetricsEnd(Metrics&);

#endif // _METRICS_H_
