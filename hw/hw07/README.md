# Homework 7 - Inspector Detector 

In this homework, you will be laying the foundation for automated scanning in Homework 8 that will detect malicious URLs as well as exfiltrated sensitive data contained within archives. You will write the underlying tools in this homework that you will build upon / improve in Homework 8.

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, November 4th |
| **Due** | Monday, November 11th - 10 PM |
| **Due (EC)** | Friday, November 8th - 10 PM |
| **Group** | Group |  
| **Canvas Link** | Homework 07 |
| **Points** | 57 |

## Task 1 - Set up Your Group Repo

As this homework will be using shell scripting, it is strongly recommended to use the student machines for development and testing.  

1. Decide which of your group members will host the repo.
2. Make sure that your group has been identified to Prof. Striegel and that a group has been set up in Canvas.  The group may be different than the group that you used for the Python group homework.  
3. It will be up to you if you wish to use branching on this homework.  Keep in mind that branching with multiple group members may be challenging for coordination.  
4. Create the appropriate homework directory (`hw07` in the `hw` directory) and a populated `README.md`. 

As needed, you may want to read up a bit on the [rebase](https://www.atlassian.com/git/tutorials/rewriting-history/git-rebase) command as well as how to [undo changes](https://www.atlassian.com/git/tutorials/undoing-changes) in git as well. Tagging via @ in the CSE Slack channel can also help with debugging git. 

## Task 2 - Understanding the Problem

The issue brought to you is that your employer is having issues with archives coming in with malicious URLs (e.g. malware sites) as well as employees still sending sensitive information out via e-mail.  While there are a large number of vendors that provide such services, funding is tight and your boss would like you to help out with a stopgap solution until a contract can be approved for a security gateway.  Your e-mail provider (obviously not Google) has worked out a way to copy all inbound e-mails with archived attachments to a pre-defined set of directories as well as to read a separate set of directories as to whether or not an e-mail is approved (or denied).  

Your assigned task is to build up a proof of concept to do cursory e-mail protection of archived attachments on both inbound as well as outbound e-mails. For this homework, you will be writing four different scripts:

* `ae.sh`: This script is responsible for extracting different kinds of archives
* `sbs.sh`: Searches a particular file for various bad sites that may be present
* `sf.sh`: Searches files for sensitive numbers (SSN or ND ID)
* `aa.sh`: Analyzes an archive for bad sites and sensitive information

Similar to Homework 3 and 4, these scripts may be an excellent way to divide the work amongst group members.

### Task 3 - Prototyping Tools

The focus in this homework will be to work with various examples and create robust, effective scripts for handling the underlying tasks for Homework 8.

### Task 3a - Archive Extraction

Look in the `examples/archives` directory for Homework 7. There are several example archives present there.  Your code should support the following archives: `.tar`, `.tar.gz`, and `.zip`.  Figure out the correct tools and their respective invocations.  

> You will not need to handle the case of nested archives, e.g. a `.zip` inside of a `.tar` by default.  That will be an extra-credit option.  

Write a script named `ae.sh` (Archive Extract) that takes in a filename as its argument and appropriately extracts the content of the file.  It should make sure to do this in a sub-directory named `archive` (creating the directory if necessary).

> You will need to make a separate sub-directory to extract the files so that it does not clobber any of our scripts that we wish to keep.  For instance, what is the zip file contained a file named `ae.sh` or `sbs.sh` and overwrote our code?  In that case, hopefully you did some `git commit` invocations prior.  

In your later code, you will need to handle the fact that multiple sub-directories may be created as part of the archive extraction process.  In prior homework and in class, we have explored how to get a listing of all of a specific type of file contained within a given path. 

> You do not need to catch the various strings in binary files.  Your search code / scripts (bad sites, sensitive numbers) should simply ignore those files.

For now, just make sure that you appropriately interpet the extension that is present and extract the file in a sub-directory named `archive`.

You might run your code like this:

```
    $ pwd
    $ /escnfs/home/striegel/repos/student-cse20289-fa24-striegel/hw/hw07
    $ ls
    ae.sh example-zip.zip
    $ sh ae.sh example-zip.zip
    archive directory is not present .. creating!
    Extracting a zip file via unzip
    $ ls
    ae.sh archive example-zip.zip
    $ cd archive
    $ ls
    Lecture09.pdf
```

A second invocation after copying `example-tar.tar` over might look like this:

```
    $ ls
    ae.sh archive example-zip.zip example-tar.tar
    $ sh ae.sh example-tar.tar
    archive directory already present - no need to create
    Extract a tar file
    $ ls
    ae.sh archive example-zip.zip example-tar.tar
    $ cd archive
    $ ls
    Lecture09.pdf badsites
    $ cd badsites
    $ ls
    bad-file.eml bad-file2.eml clean-file.eml
```

The exact specifics of the output are up to you on this command.  We will be checking that your code does the right thing, not so much what you display to the user.  We will be confirming that you do extract the content of the respective files into the `archive` subdirectory.

### Task 3b - Bad Sites

There are several example files placed on-line in the `badsites` directory on Homework 7.  The files are as follows:

* `badsite-nocmnt-10.csv` : This contains a list of the first 10 bad sites from [URLHaus](https://urlhaus.abuse.ch/) in CSV format without leading comments.
* `badsite-10.csv` : This contains a list of the first 10 bad sites from [URLHaus](https://urlhaus.abuse.ch/) in CSV format with the leading comments as denoted by #.
* `badsite-100.csv` : This contains a list of the first 100 bad sites from [URLHaus](https://urlhaus.abuse.ch/) in CSV format with the leading comments as denoted by #.

There are several example files in the `files` directory in Homework 7.

* `clean-file.eml` : A clean file that should pass.
* `bad-file.eml` : A file containing a malicious URL.
* `bad-file2.eml` : A second file containing a malicious URL.

Write a script named `sbs.sh` (Search Bad Sites) that takes in the list of bad sites as an argument and the file to search for any of the bad URLs (e.g. http://xxxx or https://xxxx).  If no bad URLs are detected, the script should echo CLEAN.  If a bad URL is detected, you should echo out MALICIOUSURL followed by the malicious URL, e.g. MALICIOUSURL, http://xxx.yyy.zzz/.

For instance, you would run your script as:

```
    $ sh sbs.sh badsite-10.csv archive/bad-file.eml
    Loaded site information .. success!
    Scanning file named archive/bad-file.eml
    Malicious URL detected!
    MALICIOUSURL: http://....
    $
```

### Task 3c - Sensitive Files

For this part, you will be writing the examples.  Draft four example files before you start your writing of the script.

* `good.txt` : A clean file that does not contain any sensitive information.
* `bad-marking.txt` : A file that trips the `*SENSITIVE*` marker.
* `bad-SSN.txt` : A file that trips the social security number (recall that SSNs have the format of XXX-XX-XXXX).
* `bad-NDID.txt` : A file that contains a Notre Dame 90x number (you may test for either a strict 900xxx number or given that many IDs now start with 902, test for 90xxxx)

Write a script named `sf.sh` (Sensitive Finder) that scans a file for sensitive information.  If no sensitive information is detected, the script should echo CLEAN.  If sensitive information is detected, you should echo out SENSITIVE followed by the reason (MARKED SENSITIVE, SSN, STUDENTID), e.g. SENSITIVE, MARKED SENSITIVE or SENSITIVE, SSN or SENSITIVE, STUDENTID.

For the ND ID, think about the format of the Notre Dame 900 number and how to test for the 900 number.  

You would run your script as:

```
    $ sh sf.sh sensitive/good.txt
    Scanning for sensitive information
    File to scan: sensitive/good.txt
    CLEAN
    $
```

### Task 3d - Bring it Together

Using your existing scripts (`ae.sh`, `sbs.sh`, `sf.sh`), write a script named `aa.sh` (Archive Analyzer) that extracts an archive and then checks any of the files within the respective archive.  It should also appropriately clean up any files extracted (e.g. you should delete the extracted files).  You may leave the `archive` directory as present but you should not include the `archive` directory as part of your repository.  

If everything is good, echo CLEAN.  Otherwise, echo out the first reason that causes the archive to be noted as potentially bad.  Note that only one reason is sufficient and that only need to identify one violation (e.g. you do not need to identify all of the files that are problematic only that there is at least one file that is problematic). Your script should suppress any output from your other scripts from appearing on the console such that only the result appears.  

### Task 3 - Wrapping Up

At this point, you should have created the following code in your group repository:

* `ae.sh` : A script to extract a variety of archives to an `archive` sub-directory (creating the directory if necessary).
* `sbs.sh` : A script to search any file to see if it contains the list of bad sites as specified as one of the input arguments.
* `sf.sh` : A script that checks for potentially sensitive information.
* `aa.sh` : A script that analyzes archives for both bad sites as well as sensitive information.  

It is OK if you have an archive sub-directory present in your working directory, just use `.gitignore` to avoid it being added to your repository. You can read up [here](https://www.atlassian.com/git/tutorials/saving-changes/gitignore) for more information on `.gitignore`.

You should make sure that you have fully committed all of your respective modifications. 

### Task 4EC - Extra Credit

If an optional argument of `-ad X` is provided (archive depth), allow for the processing of nested archives. Note that the default value for archive depth is zero and may go up to three deep.  

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

### Task 3 - 46 points

For Task 3, you have a healthy degree of freedom on the output but need to make sure to output at least a line that contains the requested information in the writeup.  

* 1 pt - All scripts are properly named
* 1 pt - Only the appropriate scripts or support files are present in the repo
* 12 pts - `ae.sh` - Archive Extraction
   * 8 pts - Operates with proper / good archive files (.tar, .tar.gz, .zip, .bz2)
   * 1 pt - Properly places files in `archive` subdirectory
   * 1 pt - Properly handles presence or lack of `archive` subdirectory
   * 2 pts - Handles various error / warning cases (no file, bad labeling of extensions, same filenames in `archive`)
* 10 pts - `sbs.sh` - Search Bad Sites
   * 6 pts - Operates with proper / well-formed site lists across a variety of files    
   * 2 pts - Properly outputs a decision
   * 2 pts - Operates robustly / shows appropriate feedback for reasonable error / warning cases (no site file, no file to assess, binary file)
* 10 pts - `sf.sh` - Sensitive number Finder
   * 2 pts - Catches SSNs across a variety of files
   * 2 pts - Catches the sensitive classification across a variety of files
   * 2 pts - Properly catches Notre Dame student ID numbers
   * 2 pts - Properly outputs a decision
   * 2 pts - Operates robustly / shows appropriate feedback for reasonable error / warning cases (no file to assess, binary file)
* 12 pts - `aa.sh` - Archive Analyzer
   * 4 pts - Operates with proper / good archive files (.tar, .tar.gz, .zip, .bz2)
   * 2 pts - Catches malicious URLs
   * 2 pts - Catches sensitive numbers
   * 4 pts - Handles reasonable errors / warnings with an appropriate output
