#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <iostream>

using namespace std;

#ifndef _METRICS_H_
#define _METRICS_H_

typedef struct Metrics {
    ulong total_time = 0; // coarse metric
    ulong build_cycles = 0;
    ulong learn_cycles = 0;
    ulong probe_and_materialize_cycles = 0;
    ulong displacement = 0;
    ulong l3Miss = 0; // coarse metric
    ulong l2Miss = 0; // coarse metric
    ulong l1Miss = 0;   // coarse metric
    ulong retiredInst = 0; // coarse metric

    // for internal use only
    struct timespec _startTime;
    struct timespec _endTime;

} Metrics;

unsigned long getTimeDiff(struct timespec, struct  timespec);
unsigned long rdpmc_core_cycles();
unsigned long rdpmc_llc_cache_misses();
unsigned long rdpmc_l2_cache_misses();
unsigned long rdpmc_l2_cache_refs();
void getMetricsStart(Metrics&);
void getMetricsEnd(Metrics&);

void getMetricsStart(Metrics&);
void getMetricsEnd(Metrics&);
void printMetrics(Metrics&);

#endif // _METRICS_H_
