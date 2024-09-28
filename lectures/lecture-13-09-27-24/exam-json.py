
'''
Remember - the triple single quote is a block comment like /* and */ in C

{ 
   "10-28-23": 
   [ 
	{ "Period" : "1Q",  "Time" : "9:42", "Scorer" : "Notre Dame",
        "What" : "Touchdown", "Notre Dame" : 7, "Pittsburgh" : 0 },
	{ "Period" : "2Q", "Time" : "7:32", "Scorer" : "Notre Dame",
       "What" : "Field Goal", "Notre Dame" : 10, "Pittsburgh" : 0 }
	... 
	{ "Period" : "4Q", "Time" : "2:30", "Scorer" : "Pittsburgh",
       "What" : "Touchdown", "Notre Dame" : 58, "Pittsburgh" : 7 }
   ],
   "10-14-23":
   [
	{ "Period" : "1Q", "Time" : "6:15", "Scorer" : "Notre Dame",
       "What" : "Touchdown", "Notre Dame" : 7, "USC" : 0 }
            ...
   ]
}
'''

# Determine the game (date) with the maximum number of pushups
def getMaxPushups (theData):

    maxDate = ""
    maxPushups = -1

    # Loop through the keys
    # Loop through the outer dictionary using the keys which are dates of the games
    for theDate in theData.keys():
        pushups = 0

        # Now we have a list, go through it and figure out when Notre Dame scored
        for theScoringEvent in theData[theDate]:
            if theScoringEvent["Scorer"] == "Notre Dame":
                pushups += theScoringEvent["Notre Dame"]


        if pushups > maxPushups:
            maxPushups = pushups
            maxDate = theDate

    return maxDate, maxPushups

    return maxDate



    for theDate, theGameData in theData.items():
