#!/bin/sh
#
# Grade homework 2 

echo "Running the tests against netid: $1"

DirCorrect="student-cse20289-fa24-$1"

echo "  Directory is $DirCorrect"

FileTest="searchsrc.py"
FileDir="searchdir.py"

# Location for tests
GradeTest="../../../grade/hw02"

# Switch to the main directory
cd ../$DirCorrect/hw/hw02

echo "== Confirming Repository Directory =="
pwd

echo "== Revised SearchSrc.py =="
echo "Provided Test Case: hw02/nrd-1/PktQueue.cc"
if [ $1 = "striegel" ] 
then
    python3 $FileDir $GradeTest/nrd-1 --file PktQueue.cc
else
    python3 $FileTest $GradeTest/nrd-1/PktQueue.cc --include --includelocal --memberfuncs --onelinefuncs 
fi
echo ""

echo "Provided Test Case: hw02/nrd-1/fmnc_manager.cc"
if [ $1 = "striegel" ] 
then
    python3 $FileDir $GradeTest/nrd-1 --file fmnc_manager.cc
else
    python3 $FileTest $GradeTest/nrd-1/fmnc_manager.cc --include --includelocal --memberfuncs --onelinefuncs 
fi
echo ""

echo "Provided Test Case: hw02/nrd-1/RIPPS_PktPair.cc"
if [ $1 = "striegel" ] 
then
    python3 $FileDir $GradeTest/nrd-1 --file RIPPS_PktPair.cc
else
    python3 $FileTest $GradeTest/nrd-1/RIPPS_PktPair.cc --include --includelocal --memberfuncs --onelinefuncs 
fi
echo ""

echo "Provided Test Case: hw02/nrd-1/Thread_IO.cc"
if [ $1 = "striegel" ] 
then
    python3 $FileDir $GradeTest/nrd-1 --file Thread_IO.cc
else
    python3 $FileTest $GradeTest/nrd-1/Thread_IO.cc --include --includelocal --memberfuncs --onelinefuncs 
fi
echo ""

echo "Provided Test Case: hw02/nrd-1/ParamDictionary.cc"
if [ $1 = "striegel" ] 
then
    python3 $FileDir $GradeTest/nrd-1 --file ParamDictionary.cc
else
    python3 $FileTest $GradeTest/nrd-1/ParamDictionary.cc --include --includelocal --memberfuncs --onelinefuncs 
fi
echo ""

echo "== searchsrc.py =="
cat $FileTest

echo "== searchdir.py =="
cat $FileDir
echo ""

echo "== Non-Recursive Directory Testing =="

echo "Provided Files: hw02/nrd-1"
python3 $FileDir $GradeTest/nrd-1 
echo ""

echo "Test Files: hw02/nrd-2"
python3 $FileDir $GradeTest/nrd-2 
echo ""

echo "Test Files: hw02/nrd-3"
python3 $FileDir $GradeTest/nrd-3 
echo ""

echo "== Recursive Directory Testing =="

echo "Provided Files: hw02/rd-1"
python3 $FileDir $GradeTest/rd-1 -r
echo ""

echo "Test Files: hw02/rd-2"
python3 $FileDir $GradeTest/rd-2 -r
echo ""

echo "Test Files: hw02/rd-3"
python3 $FileDir $GradeTest/rd-3 -r
echo ""

echo "== CSV Creation =="

echo "Provided Files - CSV: hw02/nrd-1"
python3 $FileDir $GradeTest/nrd-1 --csv Test-NRD1.csv
echo ""
cat Test-NRD1.csv
rm Test-NRD1.csv

echo "Test Files - CSV: hw02/rd-1"
python3 $FileDir $GradeTest/rd-1 --csv Test-RD1nR.csv
echo ""
cat Test-RD1nR.csv
rm Test-RD1nR.csv

echo "Test Files - CSV: hw02/rd-3"
python3 $FileDir $GradeTest/rd-3 -r --csv Test-RD3.csv --quiet
echo ""
cat Test-RD3.csv
rm Test-RD3.csv

echo "== Statistics =="

echo "Provided Files - Stats: hw02/nrd-1"
python3 $FileDir $GradeTest/nrd-1 --stats
echo ""

echo "Test Files - Stats: hw02/rd-1"
python3 $FileDir $GradeTest/rd-1 --stats
echo ""

echo "Test Files - Stats: hw02/rd-2"
python3 $FileDir $GradeTest/rd-2 --stats --quiet
echo ""

echo "== Full Run =="

echo "Full Run on ScaleBox"
python3 $FileDir $GradeTest/ScaleBox -r --stats --csv TR-SB.csv --quiet
cat TR-SB.csv
rm TR-SB.csv

echo "** Saved CSV Output"
cat fullrepo.csv

echo "** Saved Stat Output"
cat stats.txt

echo "== Robust Inputs =="

python3 $FileDir $GradeTest/ScaleOops --stats --csv TR-SB.csv --quiet -r
python3 $FileDir $GradeTest/rd-4 -r
python3 $FileDir $GradeTest/nrd-4
python3 $FileDir $GradeTest/rd-1 -what
python3 $FileDir $GradeTest/rd-2 --csv --quiet

echo ""
echo "========="

