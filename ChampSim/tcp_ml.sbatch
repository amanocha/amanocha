#!/bin/bash
#
#SBATCH -A p2017001
#SBATCH -p node
#SBATCH -N 2
#SBATCH -n 36
#SBATCH -t 06:00:00

tracedirectory=$1
prefetchdirectory=$2
prefetcher=$3
warm=$4
instructions=$5
thtlength=$6
phtways=$7
m=$8
n=$9
phtindex=${10}
phttag=${11}
replacement=${12}

for file in "$tracedirectory"/*
do
  if [[ $file =~ .*/(.*)\.trace.gz$ ]]; then
    prefix=${BASH_REMATCH[1]}
  fi

  if [[ $file =~ .*/(.*)$ ]]; then
    fileprefix=${BASH_REMATCH[1]}
  fi
  
 # eval "./bin/hashed_perceptron-no-no-no-${prefetcher}-lru-1core -prefetch_warmup_instructions ${warm}000000 -simulation_instructions ${instructions}000000 -traces $file > results/${prefix}_${prefetcher}.txt 2>&1 &"
  eval "<${prefetchdirectory}/${prefix}_${thtlength}_${phtways}_${m}_${n}_${phtindex}_${phttag}_${replacement}.txt bin/hashed_perceptron-no-no-no-${prefetcher}-lru-1core -prefetch_warmup_instructions ${warm}000000 -simulation_instructions ${instructions}000000 -traces $file > results/${fileprefix}-hashed_perceptron-no-no-no-${prefetcher}-lru-1core.txt 2>&1 &" 

  sleep 10

done

wait
