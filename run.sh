#!/bin/bash

DATA=/home/amanocha/prefetch_traces/spec06/

for trace in $DATA* ; do
  base=$(basename -- $trace)
  echo "Checking if ${trace} files exist..."
  if [[ ! -d "output/${base%.txt}/addr.txt" ]] ; then
      echo "./sim ${trace}"
      ./sim "${trace}"
  fi
  echo ""
done 
