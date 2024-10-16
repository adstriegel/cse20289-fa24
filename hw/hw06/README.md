# Homework 6 - CSE20289 - Fall 2024 

In Homework 6, we will be blending Python unit tests together with shell scripting.  For the purposes of Homework 6, we will bring back / improve your code from Homework 2, write various unit tests for your Homework 2 (including recursive and non-recursive testing), and then use various shell script wrappers for saving the testing.  

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Wednesday, October 16th |
| **Due** | Monday, November 4th - 10 PM |
| **Due (EC)** | Friday, November 1st - 10 PM |
| **Group** | Individual |  
| **Canvas Link** | Homework 06 |
| **Points** | TBA |

## Homework Overview

As noted in the introduction, in this homework you will be creating an automation script that invoke various unit tests for your Homework 2 code. At a high level, the idea is the following:

```
$ ./doTests.sh
Running the unit tests for Homework 2 ... 
  Saving the results in timestamped files: 2024-10-16_11-06-UnitTest.log
  All tests passed successfully!
```

Your shell script will be responsible for the following:

* Pull the latest version of the test subdirectory
* Invoke the appropriate unittest(s) for the code saving the result to a timestamped log file
* Display a message whether or not the unittests were passed successfully
   * If not, display the entirety of the log file to the console

Your unit tests should appropriately flex the functionality of your code from Homework 2 including the following:

* Confirm each of the various functions as written for `searchsrc.py` provide the correct result
* Confirm each of the various functions as written for `searchdir.py` provide the correct result including cases both with and without recursive directory scanning enabled
   * A set of example directories from the test code for grading is included in the class repo
* Confirm the robustness of invoking the `searchdir.py` code using a sub-process and various argument combinations

Note that for the purposes of this homework, you will not be be asked to write tests for the CSV, quiet, or statistics functionality.

## Task 1 - Set up the Homework

Begin by setting up / starting Homework 6.  

1. Start a new branch for your homework named `hw06`.  
2. Create the appropriate homework directory (`hw06` in the `hw` directory) and a populated `README.md`. Add in a `.gitignore` to make sure Python cache files are not tracked.  
3. Copy over your two Python files from Homework 2 into your `hw/hw06` directory. Rename those two files from `searchsrc.py` and `searchdir.py` to `hw6searchsrc.py` and `hw6searchdir.py`. 
4. Create two blank files `hw6searchsrc.py.bad` and `hw6searchdir.py.bad` to be used as placeholders for "buggy" versions of your code that will fail the unit tests. We will circle back to those later.

For this homework, you will not need to use the Python virtual environment.

## Task 2 - Update HW 2

To help assist with testing, the following changes should be made to your HW 2 code:

* Make sure that each of your functions are capable of returning an integer value for the various measurements in `hw6search.py` (e.g. `lines`, `include`, `includelocal`, `memberfuncs`, and `onelinefuncs`)
* Write functions for extracting both the path as well as the filename itself
* If your code does not already, you should make sure that your code uses `argparse`
* Set up your `hw6searchsrc.py` code such that it conducts a check to see if it is being invoked directly similar to Homeworks 3 and 4.  
* If you are so inclined, you may elect to rewrite your `hw6searchdir.py` code to directly import functionality from `hw6searchsrc.py` rather than calling `subprocess` though you will need to make sure that `hw6searchsrc.py` is still directly executable.  

Double check that your code is largely working correctly.

## Task 3 - Write the Unit Tests

Read / refresh your understanding of the Python `unittest` module via the following links:

* [Python unittest for functions](https://www.freecodecamp.org/news/how-to-write-unit-tests-for-python-functions/)
   * This link describes how to construct a class dedicated to testing for Python functions.
* [Writing unittests for your code](https://realpython.com/python-unittest/)
   * Note in particular the usage of the `verbosity` setting in the call to `unittest.main()`.  We will be using setting 2.
   * Also note that we will be using docstrings in the unittests.  

For the purposes of this homework, we will be using the auto-discovery features of unit test meaning that we will be invoking the unittest by writing:

```
$ python -m unittest discover
```

Read up a bit on the proper naming convention for making test files automatically discoverable (must start with `test*`).  For the purposes of this homework, we will be putting all things test related into a `tests` sub-directory off of your `hw06` directory, e.g. `hw/hw06/tests`.  Create that directory. Create a second sub-directory named `data` off of that testing sub-directory (`hw/hw06/tests/data`).  That will be the location where all of your testing reference files (source code that was evaluated) will be located.  

### Task 3a - Starting unittests for `hw6searchsrc.py`

For your first test, we will start small by writing our first unit test to measure the number of lines of code from one of the five reference source files from HW 1.  Copy over those five reference files (`fmnc_manager.cc`, etc.) into your `tests/data` sub-directory.  

Write your first unit test selecting any of those five files as a starting point.  You can use the `wc -l` command if you want to confirm the ground truth or hopefully your original HW1 code to confirm the correct answer.  

Create the test code and then confirm that your test suite can be properly invoked by running:

```
$ python -m unittest discover
```

in your `hw/hw06` directory.  You may select the filename of your choosing for the test code. 

Write two more unittests for your new (or existing) functions that extract the path and filename.  Note that unlike the first test, these do not require a live file but can be strings of your choosing though an actual path (relative or absolute) is a great starting point.  

Confirm that all three of your tests operate correctly.  

### Task 3b - Complete basic tests for `hw6searchsrc.py`

Complete your set of unit tests for `hw6searchsrc.py` with tests across each of the respective source files (the examples provided in Homework 1) for each of the respective options and each of the respective files. The naming convention that you use is up to you with the only requirement being that the name of the testing function supports what the `unittest` module is expecting. 

Altogether, you should have at least 25 tests for numeric values and 4 or more tests for the path and filename extraction. Remember to properly label your tests.  

### Task 3c - Test correct invocations of `hw6searchsrc.py` 

In this next set of tests, we will want to test correct invocations of the `hw6searchsrc.py` script directly. Think about what the various permutations for invoking `hw6searchsrc.py` should / can be for correct invocations.

* What is the minimum set of arguments that need to be present?
* Could you have some or all of the arguments?
* Could you change the order of the arguments?

As you saw with Task 3b, the number of permutations can expand quite rapidly.  For this task, try to come up with a representative set of tests (say 5 or more tests) where you have several different invocations of `hw6searchsrc.py`.  

This is also where there is a bit of a dilemma in terms of testing, namely, what is the correct output and what should you test for?  Should you look for an exact match of the formatting or a patterned match of the formatting?  Could you / should perhaps make a regular expression that captures what will be expected?  What is the expected behavior and what matters?  For instance, does it matter if you have `--include` as an argument before `--includelocal` or should it matter?  

This is largely a matter of what is needed for your code.  For the purposes of this homework (and many programs), we write those earlier unit tests to confirm the correct answer and use these later tests to confirm that the output being displayed is reasonable with the assumption that if we pass the earlier unit tests, the output numbers will be correct.  Hence, the usage of a regular expression is quite appropriate.

Since we are checking multiple lines though, this can get a bit trickier.  It is up to you how best to capture these tests.  Do you embed the required output in your test code entirely or do you go with a result template (ex. YAML) allowing you to modularize your testing even further? Much like the earlier choices, this is left as an exercise for you to choose.

> **Note:** Since we are trying to invoke the code with arguments, you will need to recall how we can do that.  We cannot necessarily import the code and because of that, we will need to think about our relative pathing between our testing directory and the code itself (e.g. run the code as a subprocess with the script being found at `../hw6searchsrc.py`).

### Task 3d - Robust Testing

Create several tests that should raise errors / be handled properly by your code.  Think about various test cases such as:

* No file is specified
* File is not present
* File is not a `.cc` file
* File is a directory
* Invalid argument is given

Write test cases to capture each of these instances.  

### Task 3 - Recap

At this point, you should have a fairly robust test suite for the single source file Python script.  If you have not done so already, make sure to commit your code to your local repository.  

## Task 4 - Test `hw6searchdir.py`

Now that you have `hw6searchsrc.py` fully tested, move on to the directory search. For the directory search, we will be testing the following sets of functionality:

* Non-recursive iterations
* Recursive iterations

This set of tests should be contained in a separate Python file appropriately named.

For a set of example files, take a look at the `ex-tests` sub-directory in the class repository.  It contains the set of example directory constructions used as part of the testing for Homework 2. `nrd-X` refers to cases where there are no sub-directories.  `rd-X` refers to directories where multiple sub-directories are present.

Remember that for Homework 2, output is summarized in a compact and pretty format meaning that you will have one line per file that is present / analyzed.  For the purpose of this set of unit tests, we will only test directly executing `hw6searchdir.py` as a subprocess.  

Write unit tests to capture each of the respective test directories with the appropriate arguments (`-r`).  We will not be testing the CSV or stats functionality for this homework.  

> **Note:** If you previously used `--r`, make sure to update your code for `-r`.

Design two more test cases where recursion is enabled.  For instance, nest a sub-directory deeper than the existing examples or create directories with files but without any source files.  

Write unit tests to also check the robustness of the code.  Confirm that the code handles bad directories (the initial path is bad) and properly flags bad arguments. 

## Task 5 - Shell Script for Automation

Your final task will be to write a shell script that wraps around all of the Python functionality.  Your shell script should have the following attributes:

* It should be invoked from your `hw06` directory
* It should properly launch the Python unit test and capture the output from the unit test into a file following the naming convention of `2024-10-16_11-06-UnitTest.log` which is the year, month, day, hour, and minute. 
   * Display where the log is going 
* The log should capture all of the unit tests and the respective outputs (stdout, stderr) 
* Determine if all of the unit tests were passed
   * If the unit tests were passed, display the message as noted earlier
   * If any of the unit tests fail, display the entire log

Once you have confirmed that everything is working, make sure to commit your changes to your local repository.

**Note**: Really, really, really - make sure to commit your changes as the next few steps can be hazardous to your progress if you do not.

## Task 6 - Make Buggy Code

Copy your working code from `hw6searchsrc.py` and `hw6searchdir.py` to `hw6searchsrc.py.good` and `hw6searchdir.py.good`. Add and commit those files just in case.

Copy those pieces of code to the filenames with the `.bad` extension to create a starting point for creating "buggy" versions of your code. 

Rename your `.bad` files to remove the `.bad` designation to make those files your current solution.  

> Now you see why we made sure to do the commit and save the files with a `.good` extension.

Introduce code such that at least three of the test cases fail across your suite of test cases. Confirm that your script manages to catch and display the log whenever some of the test cases fail.  Document in your `README.md` which bugs that you introduced and which test cases fail.  

Copy the "buggy" software so it has the `.bad` extension once again.  

Commit all of your changes to your repository.

## Submission

Make sure to appropriately clean up, comment, and format your code.  Merge back your `hw6` branch back into your `main` branch and push your changes to your GitHub repository.  Double check and confirm that all is well in your GitHub repo.  

## Rubric

To be added during fall break
 
