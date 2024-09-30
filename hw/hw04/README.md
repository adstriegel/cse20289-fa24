# Homework 4 - CSE20289 - Fall 2024 - DRAFT

Similar to Homework 2, you will be building on your Homework 3 submission to add the following capabilities: (1) YAML as a configuration mechanism, (2) Conversion of your Word output to PDF, and (3) Parallelization of your code.  

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, September 30th |
| **Due** | Monday, October 7th - 10 PM |
| **Due (EC)** | Friday, October 4th - 10 PM |
| **Group** | Should be done in the same group as Homework 4 |  
| **Canvas Link** | Homework 04 |
| **Points** | 50 |

## Overview

For Homework 4, we will be taking a similar tack to what you did with Homework 2 with your Homework 1 code, albeit with smaller changes.  The goal of the project is three-fold:

* *Read YAML*: Instead of taking in a set of arguments, there will be only the argument of a single YAML file. The YAML file will allow one to specify a list of reports to generate along with URLs and the respective year and month settings.  

* *Full PDF*: Whereas you stopped with creating a Word file in Homework 3, we will leverage several existing packages that automate the creation of a PDF from a Word file. 

* *Parallelization:* As we have added more to the process, the various steps can begin to accumulate in terms of the total time to complete. We will be using the Python [concurrent.futures](https://docs.python.org/3/library/concurrent.futures.html) package to parallelize the full pipeline.  

In short, your goal is to complete the process you started in Homework 3 and to allow for full parallelization of all of the respective tasks.  

## Task 1 - Set up Your Assignment

To keep things simple, select the same group member to host the group repository.  Make sure that a `hw/hw04` directory exists.  Copy over your submitted source files from Homework 3 to the Homework 4 directory.  Create a `README.md` with the standard information as well as a `.gitignore` to keep any unwanted intermediate files from your repository.  

Create a new source file named `gogo.py` that will house the majority of your new code for Homework 4. The file name `gogo.py` will be the one that is invoked.  You are welcome to define any other Python files as you see fit.   

## Task 2 - Modify `checktests.py`

Modify your `checktests.py` in your Homework 4 directory (not your Homework 3 directory) to properly capture if executed as a script directly.  Set up that code after parsing the arguments to then invoke a new function that provides the proper functionality.  That function will be imported into `gogo.py`. 

## Task 3 - Parse YAML

Read the [following tutorial about YAML](https://python.land/data-processing/python-yaml).

> **Note:** You should appropriately install YAML support in your CSE 20289 virtual environment and also demarcate which package you are using in your `README.md`.  

There are three example YAML files provided for you: [ex-do-one.yaml](./ex-do-one.yaml), [ex-do-three.yaml](./ex-do-three.yaml), and [ex-do-five.yaml](./ex-do-five.yaml).  You can also find the files on GitHub.

Write code in `gogo.py` that takes in a single argument (the name of the YAML file) that parses the YAML file and creates a data structure that holds the contents of the YAML file.  This configuration will primarily consist of a dictionary of tasks whereby each task has a unique identifier and each task should have a URL, a year, a month, and a start text filename.  In addition, there is a new field named `Prepend` that contains an identifier that should be placed at the start of the name for any output file in that tasks.

Your configuration parsing function should try to parse the YAML file and then should make
sure that it is reasonably well-formed and has the appropriate parameters.  Think about what it would mean. Did the YAML file load? Did it parse without issues? Is there a URL, month, year, start file, and prepend present in each task? Is month an integer?  In the event that the configuration file does not check out (any issues), your function should return `None` to denote a bad configuration.  

> Note: You do not need to confirm that the URL is valid or that the filename is valid at this point, only that they are present as parameters.  

## Task 4 - Convert Word to PDF

Read the following [guide as a reference point](https://medium.com/@alice.yang_10652/convert-word-doc-or-docx-to-pdf-with-python-a-comprehensive-guide-6c8e8b5a079a) or also [this guide](https://www.e-iceblue.com/Tutorials/Python/Spire.PDF-for-Python/Program-Guide/Conversion/Python-Convert-PDF-to-Word-DOC-or-DOCX.html) as well.

> **Note**: Similar to the usage of YAML, make sure to denote which package you are using for your PDF conversion.  Note that the code must be confirmed to run on the CSE student machine. It is highly recommended that you use the Spire.Doc package.  

Confirm that the code in `ex-src` indeed runs correctly and outputs a PDF.  Note that it is OK that your final output PDF contains the evaluation labeling. You can convert up to 10 pages in a PDF without a license, albeit with the watermark.  

## Task 5 - Automate the Full Process - Single Process

In your `gogo.py` write a function that takes in a single parameter which is a dictionary containing all of the relevant information to invoke your entire Homework 3 pipeline (e.g. one task).  This information would be drawn from the YAML configuration file that you parsed earlier in Task 3.  The single task example would be an excellent starting point.  

> **Note:** It may help to peek ahead at Task 6 to see how this function will be used.

Use `import` to bring in the function that you wrote in Task 2 and augment your Homework 3 code to allow for prepending of a string to the beginning of a two output Word files (e.g. the Word filename should start with the `Prepend` field value).

In your function inside of `gogo.py`, add in the PDF conversion functionality that you tested in Task 4 to convert over the respective Word files.  

Test your `gogo.py` code such that you can pass in the YAML file as a parameter and then will complete all of the respective tasks as specified in the YAML file.  Set up your code such that an appropriate message is displayed when each task is completed.  Tally and display the total number of tasks completed.  Test your error checking code as well.

```
% python3 gogo.py ex-do-one.yaml
Task task05 Done!
Completed 1 task(s)!
% python3 gogo.py ex-do-three.yaml
Task task05 Done!
Task task04 Done!
Task task03 Done!
Completed 3 task(s)!
% python3 gogo.py nofile.yaml
Error: nofile.yaml is not found
```

## Task 6 - Multiple Processes

For the final task for the homework, you will be using the [concurrent.futures](https://docs.python.org/3/library/concurrent.futures.html) package.  Your `gogo.py` code should allow the ability to specify a `--multi XX` argument where `XX` is the number of processors that should be in a range from 1 to 4.  

If the `--multi` argument is specified, use the [ProcessPoolExecutor and map](https://superfastpython.com/processpoolexecutor-in-python/) to call your function that you wrote in Task 5 where the list (sequence) provided to map is the list of tasks from the YAML configuration file.

## Task 7 - Compare Performance

Finally, compare and contrast the total run time of the `ex-do-five.yaml` script using 1, 2, and 4 processes.  To time the performance of your script, do the following:

```
% time python3 gogo.py ex-do-five.yaml
```

Make sure to clean up any PDFs in between your runs.  

List the run times in your `README.md` in a small table:

```
Processes   Real Time
1            X.XX
2            X.XX
4            X.XX
```

## Extra Credit Opportunity 1 - Up to 7 points

Create a Flask server running on port 40000 to 40100 (you can pick any port in this range) that looks for the `hw04` URL off of the student meachine with three parameters (the year, the month, and the interface). Return the appropriate PDF in response.

State the name of your Flask code in your `README.md`.

> **Note:** You must be on campus or connected by the VPN to test this functionality.  

**Note** We are waiting on an official confirmation on the ports.

## Submission Checklist

Before you submit your code, make sure to check the following:

* You have a `.gitignore` file to ensure that no Python caching files are included in your repository as well as any generated files
* You have a `README.md` file along with all group member names and e-mails as well as any specific quirks for your code.  
* You have four pieces of source code present: `gogo.py`, `checktests.py`, `plotdata.py`, `createreport.py` where each code can be run individually
* You have multiple commit messages that have `hw04` at the start and if you have multiple group members, at least one commit per group member

**Push your working code** to the group repository and **submit the final commit hash via Canvas**. Only the owning group member of the repository needs to submit the hash. 

> **Note:** For the purposes of homework tracking, the GitHub date will be used for determining the core lateness penalty (e.g. the 25% per day).  The hash may be submitted late to Canvas but will result in a 5% reduction to the maximum score and the homework no longer being eligible for the extra credit bonus.

## Rubric

To be posted at mid-week





