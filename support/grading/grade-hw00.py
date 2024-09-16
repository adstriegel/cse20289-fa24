# grade-hw00.py : Grading script for Homework 00

import os
import sys
import argparse


def checkfiles (directory):
    # Check for the presence of the files
    files = ['gitinfo.txt', 'hello.py', 'clever.txt', 'README.md' ]

    NumFiles = len(files)
    DetectedFiles = 0

    print('Checking for files in: ', directory)

    for file in files:
        if not os.path.exists(os.path.join(directory, file)):
            print('File: ', file, ' does not exist')
        else:
            DetectedFiles += 1

    print('  Detected: ', DetectedFiles, ' out of ', NumFiles)

    return DetectedFiles, NumFiles

def checkCommits (directory, startString):
    # Check for the presence of the commits

    DetectedCommits = 0

    print('Checking for commits in: ', directory)

    result = os.popen('git -C ' + directory + ' log --oneline').read()

    result = result.split('\n')

    for theCommit in result:
        theSplit = theCommit.split(' ')

        if len(theSplit) >= 2:
            secondSplit = theSplit[1].lower()       

            if secondSplit.startswith(startString):
                DetectedCommits += 1

    print('  Detected Commits (starting with ' + startString + '): ', DetectedCommits)

    return DetectedCommits


# Arguments and input for the grading script (same for all homework)

parser = argparse.ArgumentParser(description='Grade Homework 00')
# Directory where all of the student repositories are kept
parser.add_argument('dir', type=str, help='The directory containing the directories to process')
# The net ID of the student to grade
parser.add_argument('netid', type=str, help='The NetID of the student to grade')
# Should the repositories be updated as part of the grading process
parser.add_argument('--updaterepos', help='Update the repositories', action='store_true')

# The homework ID
HomeworkID = 'hw00'
HomeworkOffset = 'hw/' + HomeworkID

args = parser.parse_args()

# Check for the presence of the files

DirRepo = 'student-cse20289-fa24-' + args.netid

HomeworkPath = os.path.join(args.dir, DirRepo, HomeworkOffset)

checkfiles(HomeworkPath)
checkCommits(HomeworkPath, 'hw00')

