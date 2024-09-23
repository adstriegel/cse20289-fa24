import json

theData = json.loads(open('../ex-data/data-small.json').read())

print(theData)

print('')
for theTest in theData:
    print('Interface: ', theTest['interface'], ' Direction: ', theTest['direction'], '  Time: ', theTest['timestamp'], ' TPut (Mb/s) ', str(theTest['tput_mbps']))
