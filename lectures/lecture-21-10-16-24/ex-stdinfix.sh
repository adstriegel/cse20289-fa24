#!/bin/sh

set -x

# What is going on here?
#
# Deeper Discussion about POSIX / newlines here:
#   https://stackoverflow.com/questions/17268113/reading-input-files-by-line-using-read-command-in-shell-scripting-skips-last-lin/31397497#31397497
#
# Basically
#   Try to read
#   Double check if the line is actually not empty as a fall back
#
# Recall the True / False discussion about error codes

while read line || [[ -n $line ]]
do
  echo "$line"
done < "${1:-/dev/stdin}"

