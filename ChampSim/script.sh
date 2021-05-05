#!/bin/bash

directory=$1
#prefetcher=$2
numtraces=1
#warm=$3
#instructions=$4
for file in "$directory"/*
do
  if [[ $file =~ .*/(.*)\.trace.gz$ ]]; then
    prefix=${BASH_REMATCH[1]}
  fi

  if [[ $file =~ .*/(.*)$ ]]; then
    fileprefix=${BASH_REMATCH[1]}
  fi
   
   echo "$fileprefix"
   #eval "./bin/hashed_perceptron-no-no-no-${prefetcher}-lru-1core -prefetch_warmup_instructions ${warm}000000 -simulation_instructions ${instructions}0000 -traces $file > results/${prefix}-${prefetcher}.txt 2>&1 &"

  numtraces=$((numtraces-1))
  if [[ "$numtraces" -eq 0 ]]; then
    break
  fi
done


