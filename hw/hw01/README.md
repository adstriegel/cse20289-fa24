# Homework 1 - CSE20289 - Fall 2024

The focus of this homework is to practice a bit of Python programming, specifically opening up a file and scanning said file for a variety of strings.  

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, September 2nd |
| **Due** | Monday, September 9th - 10 PM |
| **Due (EC)** | Friday, September 6th - 10 PM |
| **Group** | To be done individually |  
| **Canvas Link** | [Homework 01](https://canvas.nd.edu/courses/99306/assignments/301234) |
| **Points** | TBA |

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
