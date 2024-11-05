#!/bin/sh

for TheFile in `find "$1" -name "*.txt" -type f`;
do
    echo "Processing $TheFile"
    NewFile=$(basename $TheFile | cut -d '.' -f 1).csv
    tail -n 38 $TheFile > $1/csv/$NewFile
done

