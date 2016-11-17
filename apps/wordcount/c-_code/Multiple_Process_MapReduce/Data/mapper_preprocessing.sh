#!/bin/bash
#
# Generate mapper datasets from one big dataset.
# The big dataset is the output of generate_data.sh

THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )


NUM_OF_MAPPERS=$1 # the number of mappers must be passed in order to know
                  # into how many datasets the big dataset has to be 
                  # sub-divided

DATA_PATH=$2 # the relative path of the dataset
DATA_DIRECTORY=$(dirname "${DATA_PATH}") # extract the directory path from the file path

if [ $NUM_OF_MAPPERS -lt 2 ]; then

  # no need to subdivide the big dataset
  exit 0

fi;



DATA_UNIT="${THIS_DIR}/${DATA_DIRECTORY}"
ORIGINAL_DATASET_SIZE=$(stat -c%s "${THIS_DIR}/${DATA_PATH}")
MAPPER_CHUNK_SIZE=$((ORIGINAL_DATASET_SIZE/NUM_OF_MAPPERS))


COUNTER=0
 # sub-divide the big dataset into small independent files
while [ $COUNTER -lt $((NUM_OF_MAPPERS - 1)) ];
do 
    dd if="${THIS_DIR}/${DATA_PATH}" skip=$COUNTER bs=$MAPPER_CHUNK_SIZE count=1 status=noxfer > "${DATA_UNIT}/mapper_$COUNTER.txt"

    let COUNTER=COUNTER+1
done;

# since only (N-1) mapper files are created, the last file gets all the remaining data
tail -c +$((MAPPER_CHUNK_SIZE*(NUM_OF_MAPPERS-1))) "${THIS_DIR}/${DATA_PATH}" > "${DATA_UNIT}/mapper_$COUNTER.txt"


