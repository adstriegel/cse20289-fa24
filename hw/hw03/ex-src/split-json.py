# split-json.py 

import json
import argparse

# Are we being executed directly?
if __name__ == "__main__":
    # Define the various arguments via argparse
    parser = argparse.ArgumentParser(description='Split up the JSON')
    parser.add_argument('InputJSON', type=str, help='Filename as an input')
    parser.add_argument('NumEntries', type=int, help='Number of entries to roll over')
    parser.add_argument('OutputFile', type=str, help='Filename as an output')    

    args = parser.parse_args()

    theData = json.loads(open(args.InputJSON).read())
    print('Detected ', len(theData), ' entries in the JSON file')

    if args.NumEntries == 0:
        theSmallerData = theData
    elif args.NumEntries > len(theData):
        print('Number of requested entries was too high - returning all entries')
        theSmallerData = theData
    else:
        theSmallerData = theData[:args.NumEntries]

    with open(args.OutputFile, 'w') as f:
        # Write the JSON to the file
        f.write(json.dumps(theSmallerData, indent=4))
        # Is this really needed?
        f.close()


