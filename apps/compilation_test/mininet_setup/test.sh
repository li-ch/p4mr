#!/bin/bash

# test for loop displays

prefix="SUM"
num_switches=10
sufix=( "s1.json" "s2.json")

tmp=""
for ((i=1;i<=10;i++))
do
   tmp="$tmp ${prefix}_s${i}.json"
done
echo $tmp
