#!/bin/bash

make clean && make
for d in {12000000,}
do
    echo "Dimension Cardinality |R| = ${d}"
    for r in {16,}
    do
        echo "Ratio |R|:|S| = 1:${r}"
        for z in {0,1,2,3}
        do
            echo "Zipf: ${z}"
            for s in {1,2,3,5,7,11,13,17,19,23}
            do
                cd data && python datagen.py ${z} ${d} ${r} ${s} && cd ..
                taskset -c 1 ./canonical ${s}
                echo "---------------"
                sleep 10
            done
        done
    done
    echo "!!!!!!!!!!!!!!"
done
