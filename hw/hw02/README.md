# Homework 2 - CSE20289 - Fall 2024

The focus of this homework is build on Homework 1 by incorporating more systems-oriented aspects of Python including directory scanning, subprocesses, and regular expressions.  

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, September 9th |
| **Due** | Monday, September 16th - 10 PM |
| **Due (EC)** | Friday, September 13th - 10 PM |
| **Group** | To be done individually |  
| **Canvas Link** | Homework 02 |
| **Points** | TBA |

## Preliminary Setup

Create a sub-directory in your private repository named `hw02` under the `hw` directory. Create a source file named `searchdir.py` that will serve as the core source code that should include your name and e-mail address as a comment in the first few lines of the file. Create a `README.md` file that also includes your name and e-mail address.  

Copy over the final version of your `searchsrc.py` file from Homework 1 into this same location.

Add and commit these files to your GitHub repository using the `hw02` string to start any commit messages for this homework.

Create a `.gitignore` file that will be used to mark any files that you do not wish to include in your repository.  That file may end up being blank if not needed.

Finally, clone the [ScaleBox repository](https://github.com/adstriegel/ScaleBox) in the same directory where your repositories are located (e.g. your private and group repositories).  The `ScaleBox` repository should not be cloned inside of your private repository. The five example files used in Homework 1 were all drawn from this same repository.  

## Overview

In this homework, we will be expanding the capabilities that you first created in Homework 1 to be able to operate on not only a single file but also to an entire directory or the files contained within that directory (e.g. an entire repository).  We will be leveraging the Python `subprocess` package to allow us to execute your code from Homework 1 (parsing / analyzing a single file).  In addition, we will be using the capabilities of the `os` package to interact with the filesystem, allowing you to orchestrate the execution of your code across each of the relevant files contained within a specific directory or even any sub-directory in a recursive manner.  You will also enhance your code to use regular expressions, a rather nifty mechanism for capturing various string patterns.  Finally, you will write code to leverage the computed results to extract various statistics about the underlying makeup of various files and report out those statistics to the console.

This will also be the first homework where you will need to add in a few guardrails to catch bad inputs.  We will however keep the guardrails reasonable focusing only on bad paths, not necessarily weird, esoteric bugs that can crop up with tools. We will also stay operating on only source files, specifically `.cc` files for the purpose of this homework.  

## Task 1 - Polishing / Improving your Homework 1

In Homework 1, there will be a bit of latitude as to specific behaviors to account for various uncertainty in the writeup.  In this homework, we will define the behavior a bit more specifically as well as incorporate regular expressions in to the mix. To help keep things a bit simpler, new arguments will be used to help separate past functionality from newer functionality.

Specifically, you will want to make sure that your code offers the following behaviors:

* Display the path to the file on a line (`path:`)
   * This is modified functionality from HW1
* Display the specific name of the file on a line (`file:`)
   * This is modified functionality from HW1
* Display the number of lines of code (`lines:`)
   * This is the same functionality as in HW1
* `--include` should note all included files (`include`)
   * This is the same functionality as in HW1
* `--includelocal` should identify any locally included files (quotes instead of `<>`) (`includelocal`)
   * This is new functionality beyond HW1
* `--memberfuncs` should identify member function definitions (`memberfuncs`)
   * This is a modified functionality beyond HW1.  
   * A member function has a `::` in the line
   * The first character in that line should be alphanumeric and be left-aligned (column 0)
   * You should use regular expressions for this.
* `--onelinefuncs` should identify any functions that have only a single line of content (comment, code, blank line, etc.).  Think of this as detecting a curly brace at the end of a line followed by two carriage returns followed by a line with the curly brace left aligned (`}`) (`onelinefuncs:`)
   * The assumption for a function is the same as with`--memberfuncs`.
   * You should use regular expressions to detect if the conditions occur but note that you will likely need to have multiple regular expression tests (e.g. the first line, the middle line, the last line).
   * You may assume that the last line will always have a right curly brace that is left-aligned.  
* You will not need to invoke `--ptr`, `simplefunc`, `--member`, or `--simplefuncec`.

For example, the output for `Thread_IO.cc` would be:

```
path: ../hw01/src/
file: Thread_IO.cc
lines: 149
include:  7
includelocal: 5 
memberfuncs:  0
onelinefuncs: 0
```

The output for 'PktQueue.cc' would be:

```
path: ../hw01/src/
file: PktQueue.cc
lines: 42
include:  1
includelocal: 1 
memberfuncs:  4
onelinefuncs: 4
```

**Note:** These code modifications will all take place in your `searchsrc.py` code that is located in your `hw02` directory. 

## Task 2 - Initiating a Subprocess

For the second task, we will switch over to your `searchdir.py` code.  We will focus to start on only the usage of `subprocess` to start.  All code from the second task onward will operate on `searchdir`.  

Write a function that accomplishes the following:

* Takes in two arguments, a path and a file
* Fuses the path and file together using `os.path.join`
* Runs `python3` via a `subprocess` that invokes your `searchsrc.py` code with all of the appropriate arguments specified as noted in Task 1
* Captures the output from the executed subprocess
* Constructs a dictionary element containing all of the captured information
* Returns that dictionary element

Test your code on a file from a fixed location. Tinker around and see if you prefer to use `run` or `popen`.  Make sure to print out results as you get them and explore.  Set up your calling code (e.g. the one that runs by default) to print out the content of the returned dictionary element and confirms it contains the "right" information.  

Once you have that code all working, write a function that takes in a dictionary element and prints out a _pretty_ and _compact_ version of the dictionary in the following format:

```
../hw01/src/, PktQueue.cc, 42 LOC, 1 I, 1 LI, 4 MF, 4 OLF
```

Confirm that your code works on the five example files from Homework 1.

## Task 3 - Scan a Directory (non-recursive)

Now that we have `subprocess` working, our next task is to begin to fill in and populate the actual functionality for `searchdir.py`.  Set up and use `argparse` to take in the following arguments:

* A required argument that is the directory to analyze
* `-r` A flag if included that denotes if directories should be processed recursively (default is false)
* `--csv XXX` An argument that also has a filename specified for the output (default is `none`)
* `--stats` A flag that denotes if statistics should be computed across each of the numeric fields and reported (default is false)
* `--quiet` A flag that requests the output to stay quiet (no output of the pretty, compact one-line per file from Task 2)

Write a function that does the following:

* Take in three parameters
   * Take in a path that is a directory
   * A flag denoting if the output should be quiet (default is false)
   * Take in a flag that denotes if recursion is allowed (default is false)

Write the code to scan the directory and to identify all `.cc` files.  Invoke your function from Task 2 on each of the `.cc` files to construct a dictionary containing the relevant values. As long as the `quiet` flag is false, invoke the one line (pretty, compact) function to display the results to the console.  Create a list consisting of all of the dictionaries and return the list as a result of invoking the function. 

> Test your code on a directory containing some or all of the files from Homework 1.

## Task 4 - Scan a Directory (recursively)

Add in the functionality to the function from Task 3 to handle recursively iterating across any sub-directories if the recursion flag is set.  You may assume that there are no loops in the sub-directories. You should trigger the recursive setting using the appropriate argument to your `searchdir.py` code.

When thinking about recursion, think of identifying any entries in the directory that are in and of themselves a directory.  Call your function using that new path and to scan that underlying directory and add the return results to your list.  We will discuss / review [recursion](https://www.programiz.com/python-programming/recursion) in class on Wednesday.

> To test recursion, create one sub-folder where your files are located from Homework 1.  Move or copy one of the files to that same sub-folder. 

## Task 5 - Extract a CSV

Write a function that takes in a list as generated by your code from Tasks 3 and 4 and then outputs a CSV to the specified filename.

Remember, a CSV will have the first row (line) contain the names of the various fields followed by each row (line) having the data.  Use the field names as described in Task 1 for the initial row and then output one row per line of data.  

> Test your code using either recursion or on a smaller directory.  You can display the content of the file using `more XX` where `XX` is the name of the file.  

## Task 6 - Compute Statistics

For each numeric field (lines, include, local include, memberfuncs, oneline funcs) that is given, compute the following statistics:

* Min and which file it is (ties may be broken as you see fit)
* Max and which file it is (ties may be broken as you see fit)
* Mean value
* Median value
* Standard Deviation

Use the `statistics` package to help compute the respective statistics.  Note that the `statistics` package will expect an list of values, not a list of dictionaries.  You will need to transform (e.g. create) a list containing only values for a given field (e.g. do lines of code, then includes, etc.).

Your code should output a table in the following format (assuming a directory with only `PktQueue.cc` being present):

```
Field, Min, MinFile, Max, MaxFile, Mean, Median, StdDev
lines, 42, PktQueue.cc, 42, PktQueue.cc, 42, 42, 0
includes, 1, PktQueue.cc, 1, PktQueue.cc, 1, 1, 0
localincludes, 1, PktQueue.cc, 1, PktQueue.cc, 1, 1, 0
memberfuncs, 4, PktQueue.cc, 4, PktQueue.cc, 4, 4, 0
onelinefuncs, 4, PktQueue.cc, 4, PktQueue.cc, 4, 4, 0
```

> If your code is solid and tested from the first tasks, the `statistics` output should be relatively robust.

## Task 7 - Protect Your Code

Add in a bit of protection to your code to consider the following cases:

* What if there is not a directory specified to scan?
* What if the directory specified as an input to `searchdir.py` does not exist?
* What if a directory does not contain any source files?
* What should happen if the CSV output file already exists?
* What if there is not a CSV file specified?

Some of these issues may be taken care of via `argparse`, others will require you to do a bit of testing / checking in your code.  Note that we will not be concerned with malformed source code files or with any disk access issues.  

## Task 8 - Go Big

Before embarking on the full repo, make sure to enable the `--quiet` option in your code to allow for streamlined testing.

Test your code in full by attempting to analyze the entire `ScaleBox` repository. Run your code with recursion enabled, save a CSV file named `fullrepo.csv`, and save the statistical results to a file named `stats.txt`.  You can direct the output of your code using the following command:

```
python3 searchdir.py DIRTOSCAN --quiet --r > stats.txt
```

## Offramps

As discussed in class, there are various off ramps that you can select to still get a substantial amount of the credit.  If you elect to take an offramp (e.g. skip), please make sure to denote that in your `README.md`. . 

* Skip Task 4 - Recursive Scanning
* Skip Task 5 - CSV Extraction
* Skip Task 6 - Statistical Computation
* Skip Task 7 - Input Protection
* Skip Task 8 - Note that Task 8 does require the completion of Tasks 4, 5, and 6

## Submission Checklist

Before you submit your code, make sure to check the following:

* You have a `.gitignore` file present even if it is blank
* The `ScaleBox` repository should not be present inside of your repository
* You have a `README.md` file along with your name and e-mail as well as any specific quirks for your code.  
* You have two pieces of source code present: `searchsrc.py` and `searchdir.py`
* You have two output files created from the run on ScaleBox: `fullrepo.csv` and `stats.txt`
* You have multiple commit messages that have `hw02` at the start

**Push your working code** to your private repository and **submit the final commit hash via Canvas**.

## Rubric

### General - 7 pts

* 3 pts - Homework submission is problem free without intervention required by the TA including files in the correct locations (`hw/hw02`) off of the private repository
* 3 pts - Code quality is excellent, clean and well commented code
* 1 pt - All source files + README.md contain name and e-mail

### Revised SearchSrc.py - 11 pts

* 6 pts - Passes all base files from hw01 with correct results
* 3 pts - Passes all test files
* 1 pt - Well-formed regex(es) is / are used for detecting member functions
* 1 pt - Well-formed regex(es) is / are used for detecting one line functions

### Subprocess Usage - 5 pts

* 1 pt - Invokes functions from `subprocess` package
* 2 pts - Code is clear / easy to follow on choice for invocation of subprocess
* 2 pts - Successful dictionary creation

### Non-Recursive Directory - 10 pts

* 5 pts - Provides correct answers in a directory with the files from HW1
* 5 pts - Handles two other directories correctly

### Recursive Directories - 10 pts 

* 5 pts - Test case(s) with a single sub-directory
* 5 pts - Test case(s) with multiple nested sub-directories

### CSV Creation - 10 pts

* 5 pts - Handles the non-recursive test case(s) correctly
* 5 pts - Handles the recursive test case(s) correctly

### Statistics - 10 pts

* 5 pts - Handles the non-recursive test case(s) correctly
* 5 pts - Handles the recursive test case(s) correctly