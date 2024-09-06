# Homework 1 - CSE20289 - Fall 2024

The focus of this homework is to practice a bit of Python programming, specifically opening up a file and scanning said file for a variety of strings.  

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, September 2nd |
| **Due** | Monday, September 9th - 10 PM |
| **Due (EC)** | Friday, September 6th - 10 PM |
| **Group** | To be done individually |  
| **Canvas Link** | [Homework 01](https://canvas.nd.edu/courses/99306/assignments/301234) |
| **Points** | 24 |

## Preliminary Setup

Create a sub-directory in your private repository named `hw01` under the `hw` directory. Create a source file named `searchsrc.py` that will serve as the core source code that should include your name and e-mail address as a comment in the first few lines of the file. Create a `README.md` file that also includes your name and e-mail address.  

Add and commit these files to your GitHub repository using the `hw01` string to start any commit messages for this homework.

## Overview

In this homework assignment, you will be exploring a few different aspects of Python including:

* *Reading a file:* Reading a text file (source code) for processing
* *Searching a string:* Searching the content of the text file for a series of particular characteristics or identifiers
* *Processing arguments:* Using the [*argparse*](https://docs.python.org/3/howto/argparse.html) Python library to help process various input arguments
* *Functions:* Defining as well as conditionally calling various Python functions

For the purposes of this homework, we will be processing C++ source code drawn from one of [Prof. Striegel's research projects](https://github.com/adstriegel/ScaleBox).  There are five selected files contained within the `src` sub-directory within the `hw01` folder in the class repository: `fmnc_manager.cc`, `ParamDictionary.cc`, `PktQueue.cc`, `RIPPS_PktPair.cc`, and `Thread_IO.cc`.

For your code, you will be writing code to accomplish the following:

* Take in the name of a file to process (required argument)
* Display the number of lines detected in the file
* If the `--include` argument is present, print out the number of include statements
* If the `--member` argument is present, count the number of `::` combinations that are present which contains the rough number of member functions defined
* If the `--ptr` argument is present, count the number of `->` (C++ pointer dereferences) that are present in the code
* If the `--simplefunc` argument is present, determine the number of functions that consist of only a single line of code (or a blank line).
   * You need only evaluate functions where left curly brace is on its own line and the right curly brace is on a separate line with potential line (could have code, comment, or be an empty line) in the middle  
   * For extra credit (via the `simplefuncec` argument), consider cases where the initial left curly brace need not be on its own line

You can also operate with a few assumptions to make things easier:

* The files will only be C++ source files (e.g. ending with `.cc`)
* The file will always be present - you do not need to test to see if the file exists
* The file will read without issue and will contain only ASCII characters
* For the pointer dereferencing, you can either look for `->` being present in a line and / or count the number of times that sub-string occurs
   * Note in your `README.md` as appropriate if you operate on a line by line basis or try to count all occurrences.  We will test for both and the example output files have results for both as well.  
* For the member counting, the same criterion applies.  
* A line with an `#include` will always start with `#include`
   * An include line that is commented out should not be counted
* For the `simplefunc` argument, the `PktQueue.cc` file would not have any simple functions while the `fmnc_manager.cc` code might have several.  

Your output should be done in the following manner:

```
   file: PktQueue.cc
   lines: 42
   include: 1
```

> *NOTE:* If you wrote your code and made it be a single line due to the incorrect formatting by Prof. Striegel, please note that in your `README.md`.  The intent is to have one line per output.  

> *NOTE:* The file could also include a path to the file as well.

If an argument is not specified, you should not output that particular result (e.g. -include).Your code should always display the name of the file and the number of lines in that particular file. The output if a particular argument is present should be the argument name (e.g. `include` followed by a colon followed by the number). See the example above which would have been executed in the following manner:

```
   python3 searchsrc.py PktQueue.cc --include
```

## Solving the Problem

For processing the arguments, it is recommended that you use the `argparse` library which drastically simplifies argument processing in Python.  For an example, take a look at the [shared Python code from Lecture 2](../../lectures/lecture-02-08-30-24/review-status.py). The `argparse` library allows you to assign default values if the actual argument is not specified.  

For each of the respective pieces of functionality, it may be beneficial to use various Python functions.  For example:

* Write a function named _readFile_ that takes in the name of the file as a parameter and then returns a list with each line as an entry in the list
* Write a function named _countInclude_ that takes in a list and returns the number of include statements detected

For testing purposes, several of the files are reasonable small.  Use those files as your proxy and confirm correct operation.

> **Hint 1:** Don't forget to do commits at a reasonable frequency.  Rather than one massive commit, it might be a good idea to commit as you write each function.

> **Hint 2:** If your code is not behaving correctly, it can be helpful to print out what the respective values of the different variables contain.  When in doubt, print it out.  

## Test Cases

A complete listing of the correct outputs with all options enabled can be found in the [test.txt](test.txt) file in the class repository.  The results from `Thread_IO.cc` and `PktQueue.cc` are excerpted below:

```
file:  ../../hw/hw01/src/PktQueue.cc
lines:  42
include:  1
member:  4
ptr:  0
simplefunc:  0
```

```
file:  ../../hw/hw01/src/Thread_IO.cc
lines:  149
include:  7
member:  0
ptr:  13
simplefunc:  0
```

## Submission and Grading

Your code should be submitted in the `hw/hw01` sub-directory and should contain a `searchsrc.py` file as well as a `README.md` file.  The `README.md` should contain if you are attempting the extra credit for the simple function test.  

Push your code to your private repository when it is complete and submit the hash via Canvas.

### Grading

* 3 pts - Homework submission is problem free without intervention required by the TA including files in the correct locations (`hw/hw01`) off of the private repository 
* 1 pt - `searchsrc.py` contains name and e-mail
* 1 pt - `README.md` is present and contains name plus e-mail
* 2 pts - `searchsrc.py` is well-formatted and appropriately commented
* 1 pt - File name is output correctly
* 2 pts - Lines of code is computed / output correctly
* 3 pts - `--include` is output correctly and all test cases are passed
* 3 pts - `--member` is output correctly and all test cases are passed
* 3 pts - `--ptr` is output correctly and all test cases are passed
* 5 pts - `--simplefunc` is output correctly and all test cases are passed
* 0 to 3 pts of extra credit - `simplefuncec` is output correctly and all test cases are passed