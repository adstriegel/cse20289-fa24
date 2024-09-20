
import json

theData = json.load(open('data.json'))

print(theData)

theData.sort(key=lambda x: x['age'])
print(theData)

theList = sorted(theData, key=lambda x: x['city'])
print(theList)