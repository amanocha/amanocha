#!/bin/bash

DATA=/home/amanocha/prefetch_traces/gap/

for trace in $DATA* ; do
  base=$(basename -- $trace)
  echo "Checking if ${trace} files exist..."
  if [[ ! -d "output/${base%.txt}" ]] ; then
      echo "./sim ${trace}"
      ./sim "${trace}"
  fi
  echo ""
done 
