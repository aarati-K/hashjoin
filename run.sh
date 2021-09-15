#!/bin/bash

for s in {1,2,3,5,7,11,13,17,19,23}
do
    cd data && python datagen.py ${s} && cd ..
    taskset -c 39 ./canonical ${s}
    echo "---------------"
    sleep 10
done
