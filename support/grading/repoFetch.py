# Fetch the repos given a particular CSV class list

import csv
import os
import argparse

parser = argparse.ArgumentParser(description='Fetch All Repos')

parser.add_argument('csv', type=str, help='The CSV file containing the class list')
parser.add_argument('dir', type=str, help='The directory to clone the repositories into')
parser.add_argument('mode', type=str, help='The mode to operate in (group or individual)')

args = parser.parse_args()

# Check that the location for cloning exists

if not os.path.exists(args.dir):
    print('Error: Directory for cloning repos does not exist')
    exit(-1)

# Open up the CSV file
if not os.path.exists(args.csv):
    print('Error: CSV file does not exist')
    exit(-1)

problemIDs = []

with open(args.csv, 'r') as csvfile:
    reader = csv.DictReader(csvfile)

    for row in reader:

        netid = row['NetID']
        gitid = row['GitID']

        if netid == None or gitid == None or len(netid) == 0 or len(gitid) == 0:
            continue

        if args.mode == 'group':
            repo = gitid + '/group-cse20289-fa24-' + netid
            dirLoc = os.path.join(args.dir, 'group-cse20289-fa24-' + netid)
        else:
            repo = gitid + '/student-cse20289-fa24-' + netid
            dirLoc = os.path.join(args.dir, 'student-cse20289-fa24-' + netid)

        print(dirLoc)

        if not os.path.exists(os.path.join(dirLoc)):
            print('Cloning: ', netid, ' ', repo)

            # Clone the repository
            result = os.system('git -C ' + args.dir + ' clone git@github.com:' + repo)

            if result != 0:
                print('Error: Unable to clone repository for netid: ', netid)
                problemIDs.append(netid)
        else:
            print('Pulling: ', netid, ' ', repo)

            # Clone the repository
            result = os.system('git -C ' + os.path.join(args.dir, dirLoc) + ' pull')


for theID in problemIDs:
    print('Problem with: ', theID)
