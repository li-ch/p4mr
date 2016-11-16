#!/bin/bash
#
# running MR under different settings

THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
MEASURE_DIR="${THIS_DIR}/multithread_measures"
mkdir -p "${MEASURE_DIR}"

SIZES=( "100MB" "200MB" "500MB" "1GB" "2GB" "5GB" "10GB" )
NUM_THREADS=( 3 6 12 24 )

for s in "${SIZES[@]}"
do
    for num in "${NUM_THREADS[@]}"
    do
        DATA_DIR="${THIS_DIR}/data/data_""$s""/"
        ${THIS_DIR}/multithread_MapReduce \
           ${DATA_DIR} \
           ${num} \
           > "${MEASURE_DIR}/mr_data_${s}_threads_${num}"
        # echo "${DATA_DIR}"
        echo "${s}-size data with ${num} threads MR finished"
    done
done
