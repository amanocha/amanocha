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
thtways=$6
phtways=$7
m=$8
n=$9
thtordering=${10}
phtindex=${11}
phttag=${12}
replacement=${13}
llcsets=${14}

for file in "$tracedirectory"/*
do
  if [[ $file =~ .*/(.*)\.trace.*$ ]]; then
    prefix=${BASH_REMATCH[1]}
  fi

  if [[ $file =~ .*/(.*)$ ]]; then
    fileprefix=${BASH_REMATCH[1]}
  fi
  
 # eval "./bin/hashed_perceptron-no-no-no-${prefetcher}-lru-1core -prefetch_warmup_instructions ${warm}000000 -simulation_instructions ${instructions}000000 -traces $file > results/${prefix}_${prefetcher}.txt 2>&1 &"
#  eval "<${prefetchdirectory}/${prefix}_${thtways}_${phtways}_${m}_${n}_${thtordering}_${phtindex}_${phttag}_${replacement}_${llcsets}.txt bin/hashed_perceptron-no-no-no-${prefetcher}-lru-1core -prefetch_warmup_instructions ${warm}000000 -simulation_instructions ${instructions}000000 -traces $file > results/results_${thtways}_${n}_${thtordering}_${phtindex}_${llcsets}/${fileprefix}-hashed_perceptron-no-no-no-${prefetcher}-lru-1core.txt 2>&1 &" 
    ./ml_prefetch_sim.py run $file --no-base --prefetch ${prefetchdirectory}/${prefix}_${thtways}_${phtways}_${m}_${n}_${thtordering}_${phtindex}_${phttag}_${replacement}_${llcsets}.txt --results-dir results/results_${thtways}_${n}_${thtordering}_${phtindex}_${llcsets} --num-instructions ${instructions} --num-prefetch-warmup-instructions ${warm} &
  sleep 10

done

wait
