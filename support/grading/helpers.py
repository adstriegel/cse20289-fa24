
import json
import os
import subprocess
import time
import argparse

from stat import S_IREAD, S_IRGRP, S_IROTH, S_IWUSR

# Fancy float gives a float with two decimal places
def fancyFloat (val):
    return "{:.2f}".format(val)


def parseArguments (theDescription):

    parser = argparse.ArgumentParser(description=theDescription)
    # Directory where all of the student repositories are kept
    parser.add_argument('config', type=str, help='The configuration file to use')
    # This is where all of the student repositories are located
    parser.add_argument('dir', type=str, help='The directory containing the directories to process')
    # The net ID of the student to grade
    parser.add_argument('netid', type=str, help='The NetID of the student to grade')

    # Should the repositories be updated as part of the grading process
    parser.add_argument('--updaterepos', help='Update the repositories', action='store_true')

    # Should the repositories be updated as part of the grading process
    parser.add_argument('--noprotect', help='Disable file protection for invocation and rely on the repo', action='store_true')

    args = parser.parse_args()    

    return args

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
        try:
            print('***** File: ', file, ' *****')
            print('')

            if not os.path.exists(os.path.join(theDirectory, file)):
                print('File: ', file, ' does not exist')
            else:
                with open(os.path.join(theDirectory, file), 'r') as theFile:
                    result = theFile.read()
                    print(result)

            print('')
        except Exception as e:
            print('Exception when trying to output file: ', e)


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

            if secondSplit.lower().startswith(startString):
                DetectedCommits += 1
                print('   Commit: ', theCommit)

    print('  Detected Commits (starting with ' + startString + '): ', DetectedCommits)

    return DetectedCommits

def loadConfigFile (configFile):
    print('Loading configuration at: ', configFile)

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

def conductTests (directory, netid, config, noProtect=False, Timeout=60):
    print('Conducting test sequence for ', netid)

    # Confirm that the submission is present
    theRepo = constructDirectory(directory, netid, config)
    theDirectory = os.path.join(theRepo, config['location'])

    if not os.path.exists(theDirectory):
        print('Error: Homework directory does not exist at the correct location ' + theDirectory)
        return

    os.chdir(theDirectory)

    # Construct a base listing of all of the files present before running the respectiv tests,
    #  e.g. what files are present to allow us to "cleanup" after a test is run to reset the
    #  directory to its original state
    repoBaseList = os.listdir(theDirectory)

    # The configuration for the script allows certain files to be write protected to prevent
    # student code from being able to overwrite (e.g. the student script opens up an input file
    # in write mode and blitzes the file away). The default value is to protect the specified
    # files
    if not noProtect:
        # Make files as requested to be read only (e.g. files that the student code may be opening in write only mode potentially)
        for theProtectFile in config['protectedfiles']:
            os.chmod(theProtectFile, S_IREAD | S_IRGRP | S_IROTH)

    for theTest in config['tests']:
        try:
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
                    result = subprocess.run(theCommand, cwd=theDirectory, timeout=Timeout, capture_output=True, shell=True)
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

                # Catch the timeout exception                    
                except subprocess.TimeoutExpired:
                    print('** Execution timed out after ', str(Timeout), ' s')
                    #print('  ** stdout (len=', len(result.stdout), ' bytes) **')
                    #print(result.stdout.decode('utf-8'))                
                    #print('  ** stderr (len=', len(result.stderr), ' bytes) **')
                    #print(result.stderr.decode('utf-8'))

                # Catch any other exception
                except Exception as e:
                    print('** Exception detection: ', e)
                    #print('  ** stdout (len=', len(result.stdout), ' bytes) **')
                    #print(result.stdout.decode('utf-8'))                
                    #print('  ** stderr (len=', len(result.stderr), ' bytes) **')
                    #print(result.stderr.decode('utf-8'))


                #result = subprocess.run(theCommand, cwd=theDirectory)
                #result.wait()
        except Exception as e:
            print('Exception detected during the test: ', e)

        print('Test ', theTest['id'], ' completed')  
        print('')

        try:
            postTestList = os.listdir(theDirectory)
            #print('Post Test List: ', postTestList)

            # Make sure our output directory exists
            if not os.path.exists(os.path.join(config['results'], netid, theTest['id'])):
                os.makedirs(os.path.join(config['results'], netid, theTest['id']))

            DetectedFiles = []

            # See which files were created by comparing a list of the files before the test and
            # the files present in the directory after the test was run
            for theExtraFile in postTestList:
                if theExtraFile not in repoBaseList:
                    DetectedFiles.append(theExtraFile)
                    # Figure out the full path
                    theExtraFullFile = os.path.join(theDirectory, theExtraFile)
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
        except Exception as e:
            print('Exception detected during the post test: ', e)

    if not noProtect:
        # Make files as requested to be back to read / write 
        for theProtectFile in config['protectedfiles']:
            os.chmod(theProtectFile, S_IREAD | S_IRGRP | S_IROTH | S_IWUSR)
