#!/bin/sh


# Search a directory and tell me the largest .cc or .h file that is present
# in that directory

directory=$1

find "$directory" -type f \( -name "*.cc" -o -name "*.h" \) -exec ls -l {} + | sort -k 5 -n | tail -n 1

