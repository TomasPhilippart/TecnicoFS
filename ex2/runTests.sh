#!/bin/bash

inputdir=$1
outputdir=$2
maxthreads=$3

# check arguments
[ ! -d "$inputdir" ] && { echo "Directory $inputdir doesn't exist."; exit 1; }
[ ! -d "$outputdir" ] && { echo "Directory $outputdir doesn't exist."; exit 1; }
[ ! $maxthreads -gt 0 ] && { echo "Maxthreads must be greater than 0."; exit 1; }

export GREP_COLOR='00;38;5;157'

color_stderr()(set -o pipefail;"$@" 2>&1>&3|sed $'s,.*,\e[31m&\e[m,'>&2)3>&1 # used to color stderr to red

for input in $inputdir*; do
    for numthreads in $(seq 1 $maxthreads); do
        output=$(basename "$input" .txt)-$numthreads
        echo "InputFile=$(basename "$input" .txt) NumThreads=$numthreads"
        color_stderr ./tecnicofs $input $outputdir$output.txt $numthreads | grep --color=auto "TecnicoFS.*"
    done
done