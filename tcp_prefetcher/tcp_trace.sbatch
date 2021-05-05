#!/bin/bash
#
#SBATCH -A p2017001
#SBATCH -p node
#SBATCH -N 2
#SBATCH -n 36
#SBATCH -t 24:00:00

directory=$1
thtlength=$2
phtways=$3
m=$4
n=$5
phtindex=$6
phttag=$7
replacement=$8

for file in "$directory"/*
do
  if [[ $file =~ .*/(.*)\.txt$ ]]; then
    prefix=${BASH_REMATCH[1]}
  fi
  
  eval "./prefetcher_core $file $thtlength $phtways $m $n $phtindex $phttag $replacement > results/${prefix}_${thtlength}_${phtways}_${m}_${n}_${phtindex}_${phttag}_${replacement}.txt &"
  sleep 10

done

wait
