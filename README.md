# Canonical PK-FK Hashjoin

## Overview

This is an implementation of the canonical join query:
```
SELECT *
FROM R, S
WHERE R.pk = S.fk
```
where the dimension table `R` and fact table `S` have a primary key-foreign key (PK-FK) dependency. We implement the canonical hash join algorithm, which involves building a hash table from the keys in `R`, followed by probing the hash table for the rows in `S`. We assume that `R` and `S` have two columns (key, payload) with 8-byte integer attributes (16-byte tuples).

Skew can arise in PK-FK relations when some keys occur more frequently than others in the outer relation `S`. Internally, we use [Wiscer](https://github.com/aarati-K/wiscer) to generate data with different levels of skew.

We test two hash table implementations:
* *Default hashjoin* uses the vanilla implementation of chained hash table.
* *VIP hashjoin* uses the VIP hash table to learn and adapt to the skew in the data.

## Building and Running

To compare the two hashjoin implementations, run:

```
$ ./run.sh > output.txt
$ python2 parse_results.py output.txt
Dimension Cardinality |R| = 12000000
Ratio |R|:|S| = 1:16
+--------------+---------------------------+-----------------------+--------------------------------+
| Skew level   |   Median Time Default (s) |   Median Time VIP (s) | Time VIP vs Default Hashjoin   |
+==============+===========================+=======================+================================+
| Zipf: 0      |                      18.5 |                  18.7 | +0.9%                          |
+--------------+---------------------------+-----------------------+--------------------------------+
| Zipf: 1      |                      14.2 |                  11.5 | -18.7%                         |
+--------------+---------------------------+-----------------------+--------------------------------+
| Zipf: 2      |                       3.4 |                   2.6 | -22.5%                         |
+--------------+---------------------------+-----------------------+--------------------------------+
| Zipf: 3      |                       2.7 |                   2.4 | -12.8%                         |
+--------------+---------------------------+-----------------------+--------------------------------+
```

The script `run.sh` tests 10 different datasets (generated using different random seeds) for each level of skew, and the script `parse_results.py` reports the median execution time for the two hashjoin implementations. It take approximately 6 hours for the script to finish running.

## System Requirements

1. `python-tabulate` - The `python-tabulate` library is required for the `parse_results.py` script to work.

2. *Hardware Metrics* - Collecting hardware metrics is disabled by default, and requires following the steps detailed in the [Wiscer](https://github.com/aarati-K/wiscer) repository to program the performance monitoring unit (PMU) in Intel processors. Collection of hardware metrics can be enabled by setting the flag `_COLLECT_METRICS_` in file `metrics.h`.\
`#define _COLLECT_METRICS_ 1`

3. *Intel Intrinsics* - Intel Intrinsics libraries might not be available for some platforms, in which case the flag `_INTEL_INTRINSICS_` in file `hashjoinvip.h` should be disabled.\
`
#define _INTEL_INTRINSICS_ 0
`
