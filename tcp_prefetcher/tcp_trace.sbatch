#!/bin/bash
#
#SBATCH -A p2017001
#SBATCH -p node
#SBATCH -N 2
#SBATCH -n 36
#SBATCH -t 6:00:00

directory=$1
thtways=$2
phtways=$3
m=$4
n=$5
thtordering=$6
phtindex=$7
phttag=$8
replacement=$9
llcsets=${10}

for file in "$directory"/*
do
  if [[ $file =~ .*/(.*)\.txt$ ]]; then
    prefix=${BASH_REMATCH[1]}
  fi
  
  eval "./prefetcher_core $file $thtways $phtways $m $n $thtordering $phtindex $phttag $replacement $llcsets > results/${prefix}_${thtways}_${phtways}_${m}_${n}_${thtordering}_${phtindex}_${phttag}_${replacement}_${llcsets}.txt &"
  sleep 10

done

wait
