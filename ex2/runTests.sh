#!/bin/bash

inputdir=$1
outputdir=$2
maxthreads=$3

make -i -f Makefile > /dev/null 2>&1 #make and ignore any outputs
mkdir -p $outputdir #if outputdir doesnt exist, create it

for input in $inputdir*; do
    for numthreads in $(seq 1 $maxthreads); do
        output=$(basename "$input" .txt)-$numthreads
        echo "InputFile=$(basename "$input" .txt) NumThreads=$numthreads"
        ./tecnicofs $input $outputdir$output.txt $numthreads 2>&1 | grep "TecnicoFS"
    done
done

