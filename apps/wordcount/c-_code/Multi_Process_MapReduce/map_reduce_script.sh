#!/bin/bash 

NUM_OF_MAPS=$1
NUM_OF_REDUCERS=$2
COUNTER=0
MAPPER_PROCESSES[0]=0

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
echo $((END_TIME - START_TIME)) >> mappers_time.txt 



declare -a REDUCER_PROCESSES
START_TIME=$(date +%s%6N)  # record start time (us)

# Reducers are started once the mappers are done.
let COUNTER=0
while [ $COUNTER -lt $NUM_OF_REDUCERS ]; do
  
    cat *"_reducer_"$COUNTER".txt" > input_to_reducer_cat_"$COUNTER".txt 
    sort -bnk4 input_to_reducer_cat_"$COUNTER".txt > input_to_reducer_sorted_"$COUNTER".txt
    ./reducer $COUNTER input_to_reducer_sorted_"$COUNTER".txt & # pass an index and an input file to the program
    REDUCER_PROCESSES[$COUNTER]=$! # store the process id   

    let COUNTER=COUNTER+1
done


wait $(echo ${REDUCER_PROCESSES[*]}) # wait until all the reducers end

END_TIME=$(date +%s%6N) #record end time (us)
# write the time difference (us)
echo $((END_TIME - START_TIME)) >> reducers_time.txt 
unset END_TIME START_TIME REDUCER_PROCESSES MAPPER_PROCESSES COUNTER 
