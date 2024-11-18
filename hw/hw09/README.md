# Homework 9 - Debugging C

In this homework, you will be shoring up an existing base of C code by improving the Makefile, properly fixing `.gitignore`, and identifying and fixing seg faults and memory leaks.  As we will be leaning heavily on gcc, gdb, and valgrind, this assignment will best be done directly on the student machines.

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, November 18th |
| **Due** | Monday, November 25th - 10 PM |
| **Due (EC)** | Friday, November 22nd - 10 PM |
| **Group** | Group |  
| **Canvas Link** | Homework 09 |
| **Points** | TBA |

## Task 1 - Get the Codebase

As we move forward with C coding, you will now start to get broader skeletons to help us explore deeper concepts in C.  For our last two assignments, you will need to do the following:

1. Go to where you pulled down the class public repository.  Do a `git pull` to fetch the most recent content for the class repository.
2. Copy over the contents within the `hw/hw09` sub-directory (including any sub-directories and the files) into your private repository in your `hw/hw09` sub-directory.  Use the `cp -R` command.
3. If all is correct, you should have the following files in your `hw09` directory:

```
Assignment7.md	data	packet.c	pcap-process.c	pcap-read.c
Makefile	main.c		packet.h	pcap-process.h	pcap-read.h
```

Beyond adding a `README.md`, the files listed above and the `data` directory and its respective files should be the only files present in your submission for Homework 9.

At this initial starting point, this is a great place to do your first commit prior to starting to serve as a reference point.  

## Task 2 - Understanding the Assignment

The premise for Homework 9 is this: A summer intern had started a project last year to process network capture files.  Unfortunately, while the intern seemed to get most of the code right, the code resulted in a segfault at execution.  The intern finished up and the code has sat untouched for a year. Alas, the intern (obviously not from the ND CSE program) had not yet learned `git` and only copied up the direct source code to a Box folder so the code is as is, no documentation outside of what is in the code.  

Your job is to bring the code up to speed, find / fix the segfault(s), and to make sure there are not any memory leaks in the code.

There are three components to Homework 9:

1. *Shore up the general codebase*: There are two parts to shoring up the codebase: (1) Making the `Makefile` look closer to the advanced file from class as well as adding in intermediate object file creation (e.g. create `.o` files; (2) Creating an appropriate `.gitignore` to allow for only the "proper" files to be included in the repository (e.g. not `redextract`, not `.o` files, etc.).
2. *Debug critical issues*: The code compiles successfuly but results in a segfault.  You will need to use your debugging skills including adding / modifying `printfs` and `gdb`. 
3. *Detect any memory leaks:* Use `valgrind` to determine if there are any memory leaks in the code and fix the code as needed.

Your submission will be a revised version of the code as well as commentary in the `README.md` as to what / how you fixed the respective issues.  

## Task 3 - Shoring up the Codebase

Your first task is to improve the codebase.  As noted earlier, you need to do the following:

* Modify the *Makefile* to properly use variables, use intermediate object files during compilation, and to link the code together into the final executable.  
* Create a `.gitignore` file that excludes any files that should not be placed in the repository, e.g. the final executable and the intermediate object files
* Create a `redextract-debug` target that brings in all of the relevant debugging information for testing purposes.  Think about what that might entail.

As a subtle hint, this is probably a pretty great place to do a commit as part of your assignment.

## Task 4 - Debug the Critical

The code compiles but results in a segfault when attempting to run the code.  You can invoke the code in the following manner:

```
$ ./redextract-debug data/testFile.pcap
MAIN: Initializing the table for redundancy extraction
MAIN: Initializing the table for redundancy extraction ... done
MAIN: Attempting to read in the file named data/testFile.pcap
Segmentation fault (core dumped)
$
$ ./redextract-debug data/TestSmall.pcap
MAIN: Initializing the table for redundancy extraction
MAIN: Initializing the table for redundancy extraction ... done
MAIN: Attempting to read in the file named data/TestSmall.pcap
Segmentation fault (core dumped)
```

> **Note:** Think about how you would modify the `Makefile` to make a version of the targeted `redextract` compilation where `redextract-debug` contains all of the relevant debug information.  The `Makefile` should be able to create a debug-centric executable `redextract-debug` and a non-debug-centric version named `redextract`

It is your job to figure out where / how / why the segfault is occurring.   You have a couple of ways that you can go about debugging:

* *Inspect the code*: Are there any built-in mechanisms in the code that are otherwise disabled? Could you turn them on easily?
* *Divide and conquer via printf:* Add in or enable various output statements to try to figure out which line the segfault is occurring.  Remember that in order to force output via `STDOUT`, you need a `\n` in your `printf`.  
* *Use gdb*: `gdb` allows you to set breakpoints as well as provide a better catch as to the where / why the segfault occurred. You can also tinker with `valgrind` though that is probably less useful.  
* *Output various values:* It can sometimes be helpful to output the various values contained within various variables (including members of structs).  Try to think what might make a segfault happen.  

You can modify the code as you see fit to fix the segfault provided that the code runs correctly overall.  When you fix the segfault (or segfaults - there may be more than one), you will definitely know.

Remember, a key skill for a programmer is the ability to instrument and understand existing code.  As programmers, writing new code can be fun / exciting but there is often ample effort invested in resolving existing code issues.

If all goes well, you should see the following output at the conclusion of running the code:

```
$ ./redextract data/testFile.pcap
...
File processing complete - data/testFile.pcap file read containing 430 packets with 441321 bytes of packet data
Summarizing the processed entries
Parsing of file data/testFile.pcap complete
  Total Packets Parsed:    430
  Total Bytes   Parsed:    441321
  Total Packets Duplicate: 0
  Total Bytes   Duplicate: 0
  Total Duplicate Percent:   0.00%
```

**Note:** While this class generally has a fairly liberal policy when it comes to the usage of ChatGPT and other tools, this is one assignment where you are asked to not use the tool so as to sharpen your debugging skills.  You are welcome to ask for help via Slack from the instructor and TAs.  

## Task 5 - Fix the Leaks

The final task is that the intern left a note saying that in trying to find the segfault, there probably were a few memory leaks that crept into the code.  A memory leak is basically a pointer that gets allocated (via malloc or new) but never gets deallocated / freed.      

Your job in this final task is to use `valgrind` to help identify any memory leaks. Read up on using `valgrind` and use it on the code to see if there are any leaks.  You should then fix those leaks.

## Task 6 - Document the Work

In your `README.md`, document what you did, particularly for tasks 4 and 5.  It may be helpful to look at the various changes as recorded in `git`  to help you do the documentation.  If you are working in a group, narrate which group member helped identify what issue in your `README.md`.

## Submission

**REMEMBER:** We will only be testing your code on the CSE student machines.  If you choose to develop on your Mac or Windows machine, we will not attempt to replicate your same setup.  Your code will be graded as it runs on the student machine.  

The submission will be the same procedure as past homework.  That means:

* Commit early and often as part of the homework (start your commit with `hw07`)
* Submit the hash of your final commit via Canvas
* In your `README.md`, feel free to add any additional commentary about what works or does not work with your code. 

Remember, you can look in Task 1 to see what should be present in your repository for the submission. No executables should be part of your repository once fully checked in (GitHub actually should grump at you if you do so). 

*There is no extra credit for this assignment*. 

## Rubric

### General Mechanics - 6 points

* 2 pts - No intervention required for the submission
* 1 pt - Right information in `README.MD`
* 1 pt - Right information in Canvas (commit hash)
* 1 pt - More thane one commit visible on GitHub with appropriate commit messages
* 1 pt - Group member contributions identified if working in a group

### Makefile Improvements - 5 points

* 3 pts - Proper Makefile as prescribed
* 1 pt - Proper `.gitignore`
* 1 pt - Only the correct files present in the submission courtesy of `.gitignore`

### Segfault Detection - 8 points

* 5 pts - Code runs properly without segfaults and provides correct results
* 3 pts - Proper documentation of the fixes in `README.md`

### Memory Leak Detection - 8 points

* 5 pts - Code runs without memory leaks
* 3 pts - Proper documentation of the fixes in `README.md`