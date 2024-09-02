#!/usr/bin/env python3

import sys
import os
import json

arguments = sys.argv[1:]   # Skip the name of the script

# Checking that there is only one command line argument, and that it is a valid JSON file
if len(arguments) == 1:
    if not os.path.exists(arguments[0]) or not arguments[0][-5:].lower() == '.json':
        # Run away, run away ...
        print("ERROR: file " + arguments[0] + " is invalid.")
        exit(1)
else:
    print('ERROR: invalid number of arguments.')
    exit(1)

fh = open(arguments[0], 'r')
json_data = json.load(fh)
fh.close()

# Data is loacted in a list, so we want to get the 'first item' which is all the dictionaries
counter = 0

print('\nBEACON INFORMATION\n')

for i in range (len(json_data)):
    print('factory_id:', json_data[i]['factory_id'])
    print('name:', json_data[i]['name'])
    print('hardware:', json_data[i]['hardware'])
    print('battery_level:', json_data[i]['battery_level'])
    print('battery_updated_date:', json_data[i]['battery_updated_date'])
    print('\n')
    counter += 1

print('Number of beacons present: ' + str(counter))
