# review-status.py - check up on paper reviews

import os
import sys
import csv
import argparse
import statistics

def fancyFloat (val):
    return "{:.2f}".format(val)

def fancyFloatPct (numerator, denominator):
    val = (numerator / denominator) * 100
    return "{:.2f}".format(val)


def parsefile_tpc(filename, tpcFile='none', CreateCSV=False):

    MappingHeaders = {}

    TPCInfo = {}

    NoneConfirmed = []
    SomeConfirmed = []

    # Open the TPC file
    if tpcFile != 'none':
        with open(tpcFile, 'r') as f:
            reader = csv.reader(f)

            # Skip the first line
            next(reader)

            for row in reader:
                # Construct the TPC Member Name
                MemberName = row[0]

                TPCInfo[MemberName] = {}

                # Save the last name
                TPCInfo[MemberName]['FullName'] = MemberName
                TPCInfo[MemberName]['EDAS-ID'] = row[1]
                TPCInfo[MemberName]['Affiliation'] = row[2]
                TPCInfo[MemberName]['Country'] = row[3]
                TPCInfo[MemberName]['Email'] = row[4]


    if CreateCSV:
        OutputCSV = open('TPC-Issues.csv', 'w')
        OutputCSV.write('Name,EDAS-ID,Email\n')
    else:
        OutputCSV = None


    # Open the file as a CSV - not as a dictionary reader
    with open(filename, 'r') as f:
        reader = csv.reader(f)

        # Skip the first line
        next(reader)

        # Read the real header line
        headers = next(reader)

        # Create a mapping of the headers to the index
        for i in range(12):
            MappingHeaders[headers[i]] = i

        CountTPC = 0
        CountNoneConfirmed = 0
        CountSomeConfirmed = 0
        CountAllConfirmed = 0
        TrackNoneConfirmed = []
        CountReviewsUnconfirmedNone = 0
        CountReviewsUnconfirmedSome = 0

        # Iterate through the rest of the file
        for row in reader:
            CountTPC += 1
            if len(row[MappingHeaders['confirmed']]) == 0:
                # print('WARNING: TPC ' + row[0] + ' has not confirmed any reviews')
                CountNoneConfirmed += 1
                NoneConfirmed.append(row[0])
            elif len(row[MappingHeaders['notified']]) > 0:
                CountSomeConfirmed += 1
                SomeConfirmed.append(row[0])
            else:
                CountAllConfirmed += 1

            
                        
    print('TPC Members - No Confirmations:   ' + str(CountNoneConfirmed) + ' (' + fancyFloatPct(CountNoneConfirmed, CountTPC) + '%)')

    if tpcFile != 'none':
        for theMember in NoneConfirmed:
            if theMember in TPCInfo:
                if CreateCSV:
                    OutputCSV.write(theMember + ',' + TPCInfo[theMember]['EDAS-ID'] + ',' + TPCInfo[theMember]['Email'] + '\n')
                else:
                    print('     ' + theMember + ' (' + TPCInfo[theMember]['Email'] + ')')
            else:
                print('   Warning: Cannot find ' + theMember + ' in the TPC file')



    print('TPC Members - Some Confirmations: ' + str(CountSomeConfirmed) + ' (' + fancyFloatPct(CountSomeConfirmed, CountTPC) + '%)')
    print('TPC Members - Fully Confirmed:    ' + str(CountAllConfirmed) + ' (' + fancyFloatPct(CountAllConfirmed, CountTPC) + '%)')



def parsefile_paper(filename,csvOnly=False):
    PaperCount = 0

    PaperStats = {}
    PaperStats['Data-ACC'] = []
    PaperStats['Data-Confirmed'] = []
    PaperStats['Data-Completed'] = []

    PaperStats['Track-NoneConfirmed'] = []
    PaperStats['Track-OneConfirmed'] = []
    PaperStats['Track-TwoConfirmed'] = []
    PaperStats['Track-ThreeConfirmed'] = []
    
    PaperStats['Track-NoneCompleted'] = []
    PaperStats['Track-OneCompleted'] = []
    PaperStats['Track-TwoCompleted'] = []
    PaperStats['Track-ThreeCompleted'] = []

    PaperStats['Track-DblBlindIssue'] = []

    PaperStats['Track-SevereScope'] = []
    PaperStats['Track-MaybeScope'] = []


    # Open the file as a csv dictionary
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)

#        print('Headers' + str(reader.fieldnames))
        

        for row in reader:
            # Increment the paper count
            PaperCount += 1

            # print(row)
            # Warning if the paper has nothing assigned
            if len(row['Reviews assigned, confirmed, completed']) == 0:
                print('WARNING: Paper ' + row['#'] + ' has no reviews assigned')
    
            PaperStats['Data-ACC'].append(int(row['Reviews assigned, confirmed, completed']))   


            if len(row['Reviews confirmed']) == 0:
                PaperStats['Track-NoneConfirmed'].append(row['\ufeff"#"'])
                PaperStats['Data-Confirmed'].append(0)    
            else:
                if row['Reviews confirmed'] == '1':
                    PaperStats['Track-OneConfirmed'].append(row['\ufeff"#"'])
                elif row['Reviews confirmed'] == '2':
                    PaperStats['Track-TwoConfirmed'].append(row['\ufeff"#"'])
                elif row['Reviews confirmed'] == '3':
                    PaperStats['Track-ThreeConfirmed'].append(row['\ufeff"#"'])

                PaperStats['Data-Confirmed'].append(int(row['Reviews confirmed']))    

            if len(row['Reviews completed']) == 0:
                PaperStats['Track-NoneCompleted'].append(row['\ufeff"#"'])
                PaperStats['Data-Completed'].append(0)    
            else:
                if row['Reviews completed'] == '1':
                    PaperStats['Track-OneCompleted'].append(row['\ufeff"#"'])
                elif row['Reviews completed'] == '2':
                    PaperStats['Track-TwoCompleted'].append(row['\ufeff"#"'])
                elif row['Reviews completed'] == '3':
                    PaperStats['Track-ThreeCompleted'].append(row['\ufeff"#"'])

                PaperStats['Data-Completed'].append(int(row['Reviews completed']))    

            if len(row['review: Double blind policy']) != 0:
                DblBlindRating = float(row['review: Double blind policy'])

                if DblBlindRating < 1.0:
                    PaperStats['Track-DblBlindIssue'].append(row['\ufeff"#"'])

            if len(row['review: Conference Scope']) != 0:
                ConfScopeRating = float(row['review: Conference Scope'])

                if ConfScopeRating < 2.0:
                    PaperStats['Track-SevereScope'].append(row['\ufeff"#"'])
                elif ConfScopeRating < 3.0:
                    PaperStats['Track-MaybeScope'].append(row['\ufeff"#"'])


    print('Total Active Papers: ', PaperCount)

    print('== Paper Reviews Confirmation (Mean=' + "{:.2f}".format(statistics.mean(PaperStats['Data-Confirmed'])) + ', Median=' + str(statistics.median(PaperStats['Data-Confirmed'])) + ')==')
    print('  Papers without any confirmations: ' + str(len(PaperStats['Track-NoneConfirmed'])) + ' ' + fancyFloatPct(len(PaperStats['Track-NoneConfirmed']), PaperCount)  + '%')
    print('  Papers needing two confirmations: ' + str(len(PaperStats['Track-OneConfirmed'])) + ' ' + fancyFloatPct(len(PaperStats['Track-OneConfirmed']), PaperCount)  + '%')
    print('  Papers needing one confirmations: ' + str(len(PaperStats['Track-TwoConfirmed'])) + ' ' + fancyFloatPct(len(PaperStats['Track-TwoConfirmed']), PaperCount)  + '%')
    print('  Papers with all confirmations:    ' + str(len(PaperStats['Track-ThreeConfirmed'])) + ' ' + fancyFloatPct(len(PaperStats['Track-ThreeConfirmed']), PaperCount)  + '%')

    print('== Paper Reviews Completed (Mean=' + "{:.2f}".format(statistics.mean(PaperStats['Data-Completed'])) + ', Median=' + str(statistics.median(PaperStats['Data-Completed'])) + ')==')  
    print('  Papers without any completed reviews: ' + str(len(PaperStats['Track-NoneCompleted'])) + ' ' + fancyFloatPct(len(PaperStats['Track-NoneCompleted']), PaperCount)  + '%')
    print('  Papers needing two completed reviews: ' + str(len(PaperStats['Track-OneCompleted'])) + ' ' + fancyFloatPct(len(PaperStats['Track-OneCompleted']), PaperCount)  + '%')
    print('  Papers needing one completed reviews: ' + str(len(PaperStats['Track-TwoCompleted'])) + ' ' + fancyFloatPct(len(PaperStats['Track-TwoCompleted']), PaperCount)  + '%')
    print('  Papers with all completed reviews:    ' + str(len(PaperStats['Track-ThreeCompleted'])) + ' ' + fancyFloatPct(len(PaperStats['Track-ThreeCompleted']), PaperCount)  + '%')

    print('== Paper Review Progress ==')

    ReviewsNeeded = sum(PaperStats['Data-ACC'])
    ReviewsCompleted = sum(PaperStats['Data-Completed'])

    print('  Total Reviews Needed: ' + str(ReviewsNeeded))
    print('  Total Reviews Completed: ' + str(ReviewsCompleted) + ' (' + fancyFloatPct(ReviewsCompleted, ReviewsNeeded) + '%)')

    print('== Paper Issues ==')

    # Papers with double blind issues
    print('  Papers with Double Blind Issues:         ' + str(len(PaperStats['Track-DblBlindIssue'])) + ' ' + fancyFloatPct(len(PaperStats['Track-DblBlindIssue']), PaperCount)  + '%')

    # Papers with scope issues
    print('  Papers with Severe Scope Issues   (< 2): ' + str(len(PaperStats['Track-SevereScope'])) + ' ' + fancyFloatPct(len(PaperStats['Track-SevereScope']), PaperCount)  + '%')

    PaperList = ""
    for paper in PaperStats['Track-SevereScope']:
        PaperList += paper + ', '
    if len(PaperList) > 0:
        print('     Papers: ' + PaperList)

    print('  Papers with Possible Scope Issues (< 3): ' + str(len(PaperStats['Track-MaybeScope'])) + ' ' + fancyFloatPct(len(PaperStats['Track-MaybeScope']), PaperCount)  + '%')

# Define the various arguments via argparse
parser = argparse.ArgumentParser(description='Check up on paper reviews')
parser.add_argument('--dir', type=str, help='The directory containing the files to process', default='.')
parser.add_argument('--file', type=str, help='The name of a specific file to process', default='none')
parser.add_argument('--mode', type=str, help='The processing mode (tpc or paper)', default='tpc')
parser.add_argument('--csv',help='Output result as CSV', action='store_true')
parser.add_argument('--filetpc', type=str, help='TPC master file', default='none')

args = parser.parse_args()

if args.file == 'none':
    # Directory mode - scan the directory for the 'latest' file

    # Open the directory and get the list of files
    files = os.listdir(args.dir)

    # Filter the files to only include the ones that are csv files
    files = [f for f in files if f.endswith('.csv')]

    # Sort the files by name
    files.sort()

    # Get the last file in the list
    args.file = files[-1]

    print('Processing file:', args.file)



# Process the file according to the mode
if args.mode == 'tpc':
    parsefile_tpc(args.dir + '/' + args.file, args.filetpc, args.csv)
elif args.mode == 'paper':
    parsefile_paper(args.dir + '/' + args.file)

