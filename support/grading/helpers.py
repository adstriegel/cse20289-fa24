
import json
import os
import subprocess
import time

# Fancy float gives a float with two decimal places
def fancyFloat (val):
    return "{:.2f}".format(val)

# Is there a submission for this netid?
def hasSubmission (directory, netid, config):
    # Confirm that the submission is present
    theRepo = constructDirectory(directory, netid, config)
    theDirectory = os.path.join(theRepo, config['location'])

    #print('Confirming directory present in ', theDirectory)

    if not os.path.exists(theDirectory) or not os.path.isdir(theDirectory):
        print('  Warning: Homework directory does not exist at ' + theDirectory)
        return False
    else:
        return True


def confirmSubmission (directory, netid, config):
    # Confirm that the submission is present
    theRepo = constructDirectory(directory, netid, config)
    theDirectory = os.path.join(theRepo, config['location'])

    # Check for the presence of the required files
    DetectedFiles, NumFiles = checkfiles(theDirectory, config['files'])
    print('')

    # Check for the presence of the required commits
    DetectedCommits = checkCommits(theDirectory, config['commit'])
    print('')

    return DetectedFiles, NumFiles, DetectedCommits

def echoSubmission (directory, netid, config):
    # Confirm that the submission is present
    theRepo = constructDirectory(directory, netid, config)
    theDirectory = os.path.join(theRepo, config['location'])

    print(theDirectory)

    if not os.path.exists(theDirectory):
        print('Error: Homework directory does not exist at ' + config['prefix'])
        return 

    for file in config['files']:
        print('***** File: ', file, ' *****')
        print('')

        if not os.path.exists(os.path.join(theDirectory, file)):
            print('File: ', file, ' does not exist')
        else:
            with open(os.path.join(theDirectory, file), 'r') as theFile:
                result = theFile.read()
                print(result)

        print('')


# Check for the presence of the required file names
def checkfiles (directory, files):

    NumFiles = len(files)
    DetectedFiles = 0

    print('Checking for files in: ', directory)

    for file in files:
        if not os.path.exists(os.path.join(directory, file)):
            print('File: ', file, ' does not exist')
        else:
            DetectedFiles += 1
            print('   File: ', file, ' exists')

    print('  Detected: ', DetectedFiles, ' out of ', NumFiles)

    return DetectedFiles, NumFiles

def checkCommits (directory, startString):
    # Check for the presence of the commits

    DetectedCommits = 0

    print('Checking for commits in: ', directory)

    result = os.popen('git -C ' + directory + ' log --oneline').read()

    result = result.split('\n')

    for theCommit in result:
        theSplit = theCommit.split(' ')

        if len(theSplit) >= 2:
            secondSplit = theSplit[1].lower()       

            if secondSplit.startswith(startString):
                DetectedCommits += 1
                print('   Commit: ', theCommit)

    print('  Detected Commits (starting with ' + startString + '): ', DetectedCommits)

    return DetectedCommits

def loadConfigFile (configFile):
    if not os.path.exists(configFile):
        print('Error: Configuration file does not exist')
        exit(-1)

    return json.load(open(configFile))

# Construct the location of the repository
def constructDirectory (directory, netid, config):
    if not 'prefix' in config:
        print('Error: No prefix in the configuration file')
        exit(-1)
    
    return os.path.join(directory, config['prefix'] + netid)

def conductTests (directory, netid, config):
    # Confirm that the submission is present
    theRepo = constructDirectory(directory, netid, config)
    theDirectory = os.path.join(theRepo, config['location'])

    if not os.path.exists(theDirectory):
        print('Error: Homework directory does not exist at ' + theDirectory)

    os.chdir(theDirectory)

    repoBaseList = os.listdir(theDirectory)

    #print('Repo Base List: ', repoBaseList)

    for theTest in config['tests']:
        print()
        print('*************************************************************')
        print('* Running test: ', theTest['name'])
        print()

        os.chdir(theDirectory)

        for theCommand in theTest['commands']:
#            if "$RESULTS$" in theCommand:
#                print('Detected a command with a results directory')
#                theCommand = theCommand.replace("$RESULTS$", os.path.join(config['results'], netid, theTest['id']))
#                if not os.path.exists(os.path.join(config['results'], netid, theTest['id'])):
#                    os.makedirs(os.path.join(config['results'], netid, theTest['id']))
#                continue

            if "$THEDIR$" in theCommand:
                theCommand = theCommand.replace("$THEDIR$", theDirectory)

            print('  Command:   ', theCommand)
            print()
            print('  Directory: ', theDirectory)
            print()

            #result = os.popen(theCommand).read()
            try:
                startTime = time.time()
                result = subprocess.run(theCommand, cwd=theDirectory, timeout=30, capture_output=True, shell=True)
                endTime = time.time()
                print('  ** Execution time: ', fancyFloat(endTime - startTime), ' s')
                print()
                # Results from stdout and stderr
                print('  ** stdout (len=', len(result.stdout), ' bytes) **')
                print(result.stdout.decode('utf-8'))                
                print()
                print('  ** stderr (len=', len(result.stderr), ' bytes) **')
                print(result.stderr.decode('utf-8'))
                print()
                if(result.stderr.decode('utf-8').startswith("Traceback")):
                    print("  ** Error: Traceback detected **")
                    
            except subprocess.TimeoutExpired:
                print('  Execution timed out after 30 s')
                print('  ** stdout (len=', len(result.stdout), ' bytes) **')
                print(result.stdout.decode('utf-8'))                
                print('  ** stderr (len=', len(result.stderr), ' bytes) **')
                print(result.stderr.decode('utf-8'))

            #result = subprocess.run(theCommand, cwd=theDirectory)
            #result.wait()
            
        postTestList = os.listdir(theDirectory)
        #print('Post Test List: ', postTestList)

        # Make sure our output directory exists
        if not os.path.exists(os.path.join(config['results'], netid, theTest['id'])):
            os.makedirs(os.path.join(config['results'], netid, theTest['id']))

        DetectedFiles = []

        for theExtraFile in postTestList:
            if theExtraFile not in repoBaseList:
                DetectedFiles.append(theExtraFile)
                # Figure out the full path
                theExtraFullFile = os.path.join(theDirectory, theExtraFile)

                # Move it over
                #print(' Extra file was ', theExtraFile)
                os.rename(theExtraFullFile, os.path.join(config['results'], netid, theTest['id'], theExtraFile))

        if 'expectedfiles' in theTest:
            print('  Expected Files: ', theTest['expectedfiles'])

            for theExpectedFile in theTest['expectedfiles']:
                if theExpectedFile not in DetectedFiles:
                    print('    Missing file: ', theExpectedFile)
                else:
                    print('    Correct file: ', theExpectedFile)

            if len(DetectedFiles) > 0:
                print('    Generated files - placed in [', theTest['id'], ']: ', DetectedFiles)
            else:
                print('    No generated files detected')
        else:
            print('  Expected Files: None')

            if len(DetectedFiles) > 0:
                print('    Generated files - placed in [', theTest['id'], ']: ', DetectedFiles)

        #break
