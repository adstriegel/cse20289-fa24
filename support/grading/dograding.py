
import os
import sys
import argparse
import csv
import subprocess
import helpers

parser = argparse.ArgumentParser(description='Grade - Homework')

# Directory where all of the student repositories are kept
parser.add_argument('homework', type=str, help='The identifier for the homework information')

# This is where all of the student repositories are located
parser.add_argument('dir', type=str, help='The directory containing the directories to process')

# The CSV file containing the class list
parser.add_argument('csv', type=str, help='The CSV file containing the class list')

# Allow for grading of the assignment for a single student
parser.add_argument('--netid', type=str, help='The NetID of the student to grade, grades only a single student')

args = parser.parse_args()

# Open up the CSV file
if not os.path.exists(args.csv):
    print('Error: CSV file does not exist')
    exit(-1)

problemIDs = []

# Open up the configuration file
theConfig = helpers.loadConfigFile(os.path.join(args.homework, args.homework + '-config.json'))

if theConfig == None:
    print('Error: Configuration file failed to load - exiting')
    exit(-1)

with open(args.csv, 'r') as csvfile:
    reader = csv.DictReader(csvfile)

    for row in reader:
        netid = row['NetID']

        # Ignore a blank row if present
        if netid == None or len(netid) == 0:
            continue

        # Skip if we are grading only a single student
        if args.netid != None and args.netid != netid:
            continue

        # Invoke a subprocess to run the grading script
        print('Grading '  + args.homework + ' for: ', netid)    
        result = subprocess.run('python3 grade-' + args.homework + '.py ' + os.path.join(args.homework, args.homework + '-config.json') + ' ' + args.dir + ' ' + netid, shell=True, capture_output=True)        

        #print(result.stdout.decode('utf-8'))

        if not os.path.exists(os.path.join(theConfig['results'], netid)):
            os.makedirs(os.path.join(theConfig['results'], netid))

        # Open up the output file in the graded directory
        with open(os.path.join(theConfig['results'], netid, netid + '-stdout.txt'), 'w') as f:
            f.write(result.stdout.decode('utf-8'))

        # Open up the output file in the graded directory
        with open(os.path.join(theConfig['results'], netid, netid + '-stderr.txt'), 'w') as f:
            f.write(result.stderr.decode('utf-8'))

        # Extract the name
        StudentName = row['Name']
        LastName = StudentName.split()[-1]

        os.rename(os.path.join(theConfig['results'], netid), os.path.join(theConfig['results'], LastName + ' - ' + netid))

        #result = subprocess.run('python3' + os.path.join(args.homework, args.homework + '-config.json'), 'grade-' + args.homework, args.dir, netid], cwd=args.dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        
        #result = subprocess.run('python3 grade-hw03.py hw03/hw3-config.json ~/Documents/repos/repos-group-20289-fa24 acongdon', shell=True)

