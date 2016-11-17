#!/bin/bash 

NUM_OF_MAPS=$1
NUM_OF_REDUCERS=$2
COUNTER=0
declare -a MAPPER_PROCESSES[$NUM_OF_MAPS]

# The below while loops should run a few mappers and a few reducers in order to test latency

START_TIME=$(date +%s%6N)  # record start time (us)

# Maps run first and produce output text files for reducers to reduce.
while [ $COUNTER -lt $NUM_OF_MAPS ]; do

    ./mapper $COUNTER $NUM_OF_REDUCERS mapper_"$COUNTER".txt & # pass an index, the number of reducers and an input file to the program
     MAPPER_PROCESSES[$COUNTER]=$! # store the process id     
     let COUNTER=COUNTER+1 
done

wait $(echo ${MAPPER_PROCESSES[*]}) # wait until all the mappers end


# write the time difference (us)
END_TIME=$(date +%s%6N) #record end time (us)
echo $((END_TIME - START_TIME)) > mappers_time.txt 



# preprocess data first (the following loop just prepares data for sending it to the reducers) 
let COUNTER=0
while [ $COUNTER -lt $NUM_OF_REDUCERS ]; do
    
    START_MERG_AND_SORT=$(date +%s%6N)
    cat *"_reducer_"$COUNTER".txt" > input_to_reducer_cat_"$COUNTER".txt 
    sort -bnk4 input_to_reducer_cat_"$COUNTER".txt > input_to_reducer_sorted_"$COUNTER".txt
    END_MERG_AND_SORT=$(date +%s%6N)
    echo $((END_MERG_AND_SORT - START_MERG_AND_SORT)) >> reducers_input_preprocessing_time.txt    

    let COUNTER=COUNTER+1
done;


# the following loop starts all reducers and measures their processing time
declare -a REDUCER_PROCESSES[$NUM_OF_REDUCERS]
START_TIME=$(date +%s%6N)  # record start time (us)

# Reducers are started once the mappers are done.
let COUNTER=0
while [ $COUNTER -lt $NUM_OF_REDUCERS ]; do
    
    ./reducer $COUNTER input_to_reducer_sorted_"$COUNTER".txt & # pass an index and an input file to the program
    REDUCER_PROCESSES[$COUNTER]=$! # store the process id   

    let COUNTER=COUNTER+1
done;


wait $(echo ${REDUCER_PROCESSES[*]}) # wait until all the reducers end

END_TIME=$(date +%s%6N) #record end time (us)
# write the time difference (us)
echo $((END_TIME - START_TIME)) > reducers_total_time.txt 
unset END_TIME START_TIME REDUCER_PROCESSES MAPPER_PROCESSES COUNTER 
