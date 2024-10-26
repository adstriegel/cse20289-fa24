
import os
import sys
import argparse
import helpers


parser = argparse.ArgumentParser(description='Grade Homework 03')
# Directory where all of the student repositories are kept
parser.add_argument('config', type=str, help='The configuration file to use')
# This is where all of the student repositories are located
parser.add_argument('dir', type=str, help='The directory containing the directories to process')
# The net ID of the student to grade
parser.add_argument('netid', type=str, help='The NetID of the student to grade')

# Should the repositories be updated as part of the grading process
parser.add_argument('--updaterepos', help='Update the repositories', action='store_true')
args = parser.parse_args()

# Load the configuration file
theConfig = helpers.loadConfigFile(args.config)

print('Grading Homework 03 for: ', args.netid)

print('')
print('-- General Submission Information --')

if not helpers.hasSubmission(args.dir, args.netid, theConfig):
    print('  Warning: No submission found for ', args.netid, ' -> see other group members')
    exit(-1)

helpers.confirmSubmission(args.dir, args.netid, theConfig)
helpers.echoSubmission(args.dir, args.netid, theConfig)
helpers.conductTests(args.dir, args.netid, theConfig)





