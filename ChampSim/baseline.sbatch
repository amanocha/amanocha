#!/bin/bash
#
#SBATCH -A p2017001
#SBATCH -p node
#SBATCH -N 2
#SBATCH -n 36
#SBATCH -t 06:00:00

directory=$1
prefetcher=$2
warm=$3
instructions=$4
for file in "$directory"/*
do
  if [[ $file =~ .*/(.*)\.trace.gz$ ]]; then
    prefix=${BASH_REMATCH[1]}
  fi

  if [[ $file =~ .*/(.*)$ ]]; then
    fileprefix=${BASH_REMATCH[1]}
  fi
  
 # eval "./bin/hashed_perceptron-no-no-no-${prefetcher}-lru-1core -prefetch_warmup_instructions ${warm}000000 -simulation_instructions ${instructions}000000 -traces $file > results/${prefix}_${prefetcher}.txt 2>&1 &"
  eval "./bin/hashed_perceptron-no-no-no-${prefetcher}-lru-1core -prefetch_warmup_instructions ${warm}000000 -simulation_instructions ${instructions}000000 -traces $file > results/${fileprefix}-hashed_perceptron-no-no-no-${prefetcher}-lru-1core.txt 2>&1 &"
  sleep 10

done

wait

