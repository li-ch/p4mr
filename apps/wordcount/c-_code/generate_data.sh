#!/bin/bash
#
# Generate data files for MapReduce applications

THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

gen_file_by_size() {
    # Get the data unit file and size stats
    DATA_UNIT="${THIS_DIR}/data/Hadoop-WordCount/input/Word_Count_input.txt"
    SIZE=$(stat -c%s "${DATA_UNIT}")

    # Copy the unit file 
    mkdir -p "${THIS_DIR}/data/data_""$1"
    FILE_NAME="${THIS_DIR}/data/data_""$1""/data_""$1"".txt"
    cp ${DATA_UNIT} "${FILE_NAME}"

    # Iterate till size requirement satisfied
    while [ $SIZE -lt $2 ]; do 
        cat ${DATA_UNIT} >> "${FILE_NAME}"
        SIZE=$(stat -c%s "${FILE_NAME}")
    done;
    ls -lh "${FILE_NAME}"
    # echo "${FILE_100MB} is already around 100MB";
}

# different sizes
Bytes_100MB=$[100*1024*1024]
Bytes_200MB=$[200*1024*1024]
Bytes_500MB=$[500*1024*1024]
Bytes_1GB=$[1*1024*1024*1024]
Bytes_2GB=$[2*1024*1024*1024]
Bytes_5GB=$[5*1024*1024*1024]
Bytes_10GB=$[10*1024*1024*1024]

# generate data according to sizes
gen_file_by_size "100MB" "${Bytes_100MB}"
gen_file_by_size "200MB" "${Bytes_200MB}"
gen_file_by_size "500MB" "${Bytes_500MB}"
gen_file_by_size "1GB" "${Bytes_1GB}"
gen_file_by_size "2GB" "${Bytes_2GB}"
gen_file_by_size "5GB" "${Bytes_5GB}"
gen_file_by_size "10GB" "${Bytes_10GB}"
