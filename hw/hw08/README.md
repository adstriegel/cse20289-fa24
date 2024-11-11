# Homework 8 - Protect the Castle

In this homework, you will be building on the scripts that you wrote in Homework 7 to create an automated scanning tool that monitors a directory and appropriately confirms whether or not archives passed as e-mail attachments are approved or quarantined.

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, November 11th |
| **Due** | Monday, November 18th - 10 PM |
| **Due (EC)** | Friday, November 15th - 10 PM |
| **Group** | Group |  
| **Canvas Link** | Homework 08 |
| **Points** | 41 |

## Task 1 - Understanding the Assignment

With your proof of concept tools built in Homework 7, your task now is to fully automate the toolset.  You may choose to reuse some or all of your code from Homework 7 as appropriate.

In this homework, you may assume that e-mail attachments will be copied into a specified directory (e.g. `toscan`) that will be specified as an argument to your script. Attachments that are approved to be received or sent should be placed into an `approved` directory whereas e-mail attachments that should be reviewed should be placed into a `quarantined` directory.  
The e-mail provider will "automatically" copy files into the specified directory.  The extensions for the files are assumed to be marked accurately but your code should tolerate / not crash if the extensions are inaccurate.  

Your script should do the following:

* Monitor a specified directory for new archives
* If a new archive is detected, extract the archive somewhere  
   * If the file is not an approved archive, you may simply delete the file
* Scan any files inside of the archive for any occurrences of malicious sites
* Scan any files for any occurrences of markings that would deem the content sensitive
* Place the original archive either into the `approved` or `quarantined` directory with the same name
   * For any attachments that are marked to be quarantined, you should include an accompanying `.reason` file (keeping the same original name) that contains the reason for quarantine.  
* Continue to the next archive or if there are no archives for the moment, sleep for one second and repeat the process

## Task 2 - Scanning 

Create five directories for the purposes of this homework outside of your repository:

* `scandata`: This is the upper level directory where your data operations will take place
* `scandata\toscan`: This is the directory where files for scanning will be staged
* `scandata\approved`: This is the directory where approved attachments will be placed 
* `scandata\quarantined`: This is the directory where quarantined files will be placed along with the accompanying `.reason` file
* `scandata\log`: This is the location where the log files will be placed

### Task 2a - Feeding the Scanner

Since we do not have the ability to easily feed in files, you will need to write test scripts to copy in files.  Your test scripts should do something like the following:

    Copy an archive to the toscan directory 
    Sleep for a brief period of time (e.g. 1 or 2 seconds)
    Copy the next archive to the toscan directory
    Repeat until done
    
You should write several scripts that copy in files that are both fully good, fully bad, as well as various mixes of files.  Put these support scripts into a `test` directory.  Include documentation into your `README.md` about what each of the files accomplish for testing purposes.

**This is just for testing.  It is up to you what order or combination you want to use. The idea is that you are mimicking something putting things into the `toscan` directory.**

### Task 2b - Automating the Scanner

Expanding upon the earlier description, your script named `scanner.sh` should do the following:

* Take in as an argument the following:
   * The directory where content (archives) is located
   * The directory to place approved content (typically `approved`)
   * The directory to place quarantined content (typically `quarantined`)
   * A directory for a log that appends any decisions made by your script, creating the log if needed
   * The file containing the list of malicious URLs
* Verify that each of the input arguments are correct / viable
* Loop (forever)
   * Look for new files in the directory where new content is placed
   * If there is no new content
      * Sleep for one second and repeat the loop
   * If there is new content
      * Extract the archive in an appropriate location of your choosing
         * If the extraction files, the archive should be quarantined with the reason of `CANNOTEXTRACT`   
      * Scan the underlying files for malicious URLs - use the 100 entry CSV at a location you choose
      * Scan the underlying files for sensitive content
      * If either the malicious URL or sensitive content is violated, place the archive into the quarantine location.  Add in a file with the `.reason` extension that contains the reason for quarantine (e.g. if `FA34ECA4.tar.gz` trips an issue, there should be a file named `FA34ECA4.tar.gz.reason` created with the reason.   
      * If the archive passes, place the archive in the `approved` location.

Your code should continue to loop until exited via Control-C.

A `.reason` file should have the following format:

    Filename
    Reason    
    Trigger

The `Filename` is the file within the archive that triggered your scanner.  The list of reasons are: `CANNOTEXTRACT`, `MALICIOUSURL`, `SENSITIVE`.  The `Trigger` should be the specific trigger (the specific URL detected, MARKED SENSITIVE, SSN).    

The log functionality should operate in the following manner:

* Be named for the date on which the log was created in `YYYY-MM-DD.log` format
* Operate in an append manner
* All log entries should be a single line 
* All entries should have an appropriate date and time.
* Note when the script is started
* Note when the script is exited (you will need to catch the Control-C signal and handle it)
* Note when any archive is processed and the result (APPROVE or QUARANTINE). Include the archive name and the result. 
   * For a clean entry, that should just be DateTime, ArchiveName, APPROVE.
   * For a bad entry, that should echo the reason file together with the clean information on a single line, e.g. DateTime, ArchiveName, QUARANTINE, SENSITIVE, SSN.    

## Extra Credit - REST API Interface - Up to 15 points

Using Flask, create a server that allows one to post a file and have the file placed in the specified directory for scanning.  The web server should then appropriately identify the result and post the result as a web response.

## Submission

**REMEMBER:** We will only be testing your code on the CSE student machines.  If you choose to develop on your Mac or Windows machine, we will not attempt to replicate your same setup.  Your code will be graded as it runs on the student machine.  

The submission will be the same procedure as with earlier assignments.  That means:

* Commit early and often as part of the assignment
* When you are done, if you have used branching, checkout the main and merge your branch back into main
* Push your changes when you are absolutely done
* Submit the hash of your final commit via Canvas
* In your `README.md`, feel free to add any additional commentary about what works or does not work with your code. You should also note if you are attempting the extra credit.  

## Rubric  

### General Mechanics - 11 points

* 2 pts - No intervention required for the submission
* 1 pt - Right information in `README.MD`
* 1 pt - Right information in Canvas (commit hash)
* 1 pt - Three or more commits visible on GitHub
* 2 pts - Inclusion of only the relevant source code / correct usage as needed of `.gitignore` or selective inclusion of files
* 4 pts - Good / well-structured code

### Scanner Operation - 30 points
 
* 1 pt - All scripts for Task 4 are placed in the correct location
* 1 pt - Only the appropriate scripts or support files are repository
   * It is OK for instance to have a sub-directory where you place the bad site listing (make sure to use the 100 URL one) 
* 2 pts - Proper usage of arguments
* 8 pts - Proper operation the main loop with various test cases
* 5 pts - Proper tagging / flagging of quarantined items
* 5 pts - Proper logging / appending to the log
* 5 pts - Error / robust handling including aspects such as insufficient arguments, bad locations for directories, log files, bad archives, etc. 
* 3 pts - Proper handling of exit (Control-C)