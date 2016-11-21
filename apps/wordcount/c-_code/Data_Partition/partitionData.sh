#!/bin/bash
#
# Run dataset partition and measure how much time it takes

THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
MEASURE_FILE="${THIS_DIR}/time_to_partition.csv"
PARTITION_SCRIPT="${THIS_DIR}/splitData"
INIT_SCRIPT="${THIS_DIR}/initLogFile"

# Initialize the log file
$INIT_SCRIPT $MEASURE_FILE

SIZES=( "100MB" "200MB" "500MB" "1GB" "2GB" "5GB" )
#SIZES=( "100MB" "200MB" )
CHUNK_SIZES=( "1500" "9000" ) # 1500 bytes and 9000 bytes


for s in "${SIZES[@]}"
do
    for chunk_size in "${CHUNK_SIZES[@]}"
    do
        DATA_FILE="${THIS_DIR}/data_""$s""/data_""$s"".txt"
        $PARTITION_SCRIPT $chunk_size $s $DATA_FILE $MEASURE_FILE 
        echo "${s}-size data with ${chunk_size} chunk_size is finished"
    done
done
