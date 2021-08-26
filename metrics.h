#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

using namespace std;

#ifndef _METRICS_H_
#define _METRICS_H_

typedef struct Metrics {
    ulong build_time = 0;
    ulong probe_and_materialize_time = 0;
    ulong build_cycles = 0;
    ulong probe_cycles = 0;
    ulong materialize_cycles = 0;
    ulong displacement = 0;
    // ulong l3Miss_total; // will see later
} Metrics;

unsigned long getTimeDiff(struct timespec, struct  timespec);
unsigned long rdpmc_core_cycles();
unsigned long rdpmc_llc_cache_misses();

// void getMetricsStart(Metrics&);
// void getMetricsEnd(Metrics&);

#endif // _METRICS_H_
