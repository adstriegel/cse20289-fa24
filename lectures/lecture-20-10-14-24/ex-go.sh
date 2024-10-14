#!/bin/sh
# Example file of scripting for class

# Debugging assistance - uncomment to enable
#set -x
# Enable error exiting
#set -e

echo '-- cat --'
cat lecture-20-recap.md

echo '-- to a variable --'

echo '-- stat commands --'
stat lecture-20-recap.md
stat lecture-20-recaps.md

echo '-- echo + stat command --'
echo `stat lecture-20-recap.md`

echo '-- variable setting --'
theWC=`wc -l lecture-20-recap.md`
echo $theWC

echo '-- testing things --'
if stat lecture-20-recap.md > /dev/null 2>&1; then
    echo 'Yes indeed - the file lecture-20-recap.md is present'
else
    echo 'Negative ghost rider - not present'
fi

echo '-- Testing - different logic --'
if ! stat lecture-20-recaps.md > /dev/null 2>&1; then
    echo 'Negative ghost rider - the file lecture-20-recaps.md is not present'
else
    echo 'Yes - the file is present'
fi

echo '-- Testing - different logic --'
if ! stat lecture-20-recap.md > /dev/null 2>&1; then
    echo 'Negative ghost rider - the file lecture-20-recap.md is not present'
else
    echo 'Yes - the file lecture-20-recap.md is present'
fi


echo '-- Redirected Stat --'
theStat=`stat lecture-20-recaps.md > /dev/null 2>&1`
echo theStat
echo $theStat

echo '-- Extra Stat --'
theStatExtra=`stat lecture-20-recaps.md 2> /dev/null`
echo theStat
echo $theStat

