
import os
import sys
import argparse
import helpers

# Parse the arguments
args = helpers.parseArguments('Grade Homework 04')

# Load the configuration file
theConfig = helpers.loadConfigFile(args.config)

print('Grading Homework 04 for: ', args.netid)

print('')
print('-- General Submission Information --')

if not helpers.hasSubmission(args.dir, args.netid, theConfig):
    print('  Warning: No submission found for ', args.netid, ' -> see other group members')
    exit(-1)

helpers.confirmSubmission(args.dir, args.netid, theConfig)
helpers.echoSubmission(args.dir, args.netid, theConfig)
helpers.conductTests(args.dir, args.netid, theConfig, noProtect=args.noprotect)





