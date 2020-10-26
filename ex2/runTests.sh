#!/bin/bash

inputdir=$1
outputdir=$2
maxthreads=$3

mkdir -p $outputdir # if outputdir doesnt exist, create it. ignore errors if it already exists
export GREP_COLOR='00;38;5;157'

color_stderr()(set -o pipefail;"$@" 2>&1>&3|sed $'s,.*,\e[31m&\e[m,'>&2)3>&1 # used to color stderr to red

for input in $inputdir*; do
    for numthreads in $(seq 1 $maxthreads); do
        output=$(basename "$input" .txt)-$numthreads
        echo "InputFile=$(basename "$input" .txt) NumThreads=$numthreads"
        color_stderr ./tecnicofs $input $outputdir$output.txt $numthreads | grep --color=auto "TecnicoFS.*"
    done
done