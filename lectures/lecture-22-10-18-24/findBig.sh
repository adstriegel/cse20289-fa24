#!/bin/sh

# Enable debugging
set -x

theFiles=`find $1 -name "*.cc" -o -name "*.h"`

biggestFile=""
biggestSize=0

for theFile in $theFiles; do
    # Mac OS X specific commands
    statResult=`stat -f "%z" $theFile`
    if [ $statResult -gt $biggestSize ]; then
        biggestSize=$statResult
        biggestFile=$theFile
    fi
done

echo "The biggest file is `basename $biggestFile` which is $biggestSize bytes long"
echo "  The file is located at $biggestFile"
