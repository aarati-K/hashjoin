#!/bin/bash


for s in {1,2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89}
do
    cd data && python datagen.py ${s} && cd ..
    taskset -c 38 ./canonical ${s}
    echo "---------------"
done
