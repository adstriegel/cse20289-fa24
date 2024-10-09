# Homework 5 - CSE20289 - Fall 2024 

In Homework 5, we will be shifting gears to look at UNIX shell commands.  For this homework and the next homework, the homework will be done individually. This homework will also have you using `git branch` to start to give you experience with branching.

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, October 7th |
| **Due** | Monday, October 14th - 10 PM |
| **Due (EC)** | Friday, October 11th - 10 PM |
| **Group** | Individual |  
| **Canvas Link** | Homework 05 |
| **Points** | 14 |

## Overview - Git and Branching 

In this homework, you will be using a branch for the homework rather than working in the the `main` (default) branch. While organizational policies / approaches may vary, branches can be quite helpful in established code as they allow you to branch off when adding a feature or refining a particular aspect of functionality while preserving the ability to rapidly pivot back to working on the main or other branches as needed.  

[Read this tutorial on branching](https://www.atlassian.com/git/tutorials/using-branches).

For this homework, you will start off your homework by creating a branch and then when you are done with your homework, you will merge your branch back into the main branch before you push it to Github. 

A few pointers / tips:

* Whenever you are working a in a repo with branches, always check / confirm what branch you are in. To start, you will be in the main branch since there is only one branch in your code.  You can check which branch the local repository thinks your code is in with the `git status` command.
* For the homework, make sure to use the proper naming convention for your branch for the homework.  Your branch should be named `netid/hw05`.  One you have confirmed that you are in the `main` branch, create and check out your branch.  The `git checkout -b netid/hw05` command creates a branch and checks out that branch. 
* The same `tagging` rules apply for commits.  
* Remember to add files and to make frequent commits.
* When you are done, you will merge your changes in the branch back into the main branch. 

## Tasks

1. Create the appropriate branch for the assignment

2. Create the `hw05` sub-directory inside of your `hw05`

3. Create your normal `README.md` file

4. Using `echo`, create a file named `NETID` and put your netid into that file (e.g. the file should be named NETID and your netid should be present).  

5. Copy over the `Out-Dump.txt` file from the class repository that is in `hw05`.

   > It is up to you if you want to include `Out-Dump.txt` in your repository.  

6. Using `wc -l`, measure the number of lines present in `Out-Dump.txt`.  Remember that value and edit a file named  `theLines.txt` using [`vim`](https://www.freecodecamp.org/news/vim-beginners-guide/).
  
7. Using re-direction, place the first 10 lines of the `Out-Dump.txt` into a file named `output-startFile.txt`.  

8. Using re-direction, place the last 20 lines of the `Out-Dump.txt` into a file named `output-endFile.txt`.  

9. Copy over the `Test-Small.pcap` file from the class repository.  

   > See above with respect to the file needing to be checked in as part of your repository.  
   
10. Browse the `man` page for `xxd`.  Try to view the first 100 bytes from the `Test-Small.pcap` file.  

11. Using `xxd` with re-direction, place the output of the first 100 bytes into a file named `output-xxd.txt`.

12. Figure out the IP address for one of the student machines (e.g. `student10.cse.nd.edu`) using `nslookup`.  

13. Do a reverse DNS lookup using `nslookup` on the IP address that you learned about from the previous step.  Place the output of that command into a file named `output-reverse.txt`.

14. Figure out the IP addresses for `nd.edu` and `cse.nd.edu` and a website of your choosing.  Create a small file using `vi` named `revND.txt` and list the hostnames that come back when using `nslookup` for `nd.edu` and `cse.nd.edu`. Each line of the file should have the following: the original hostname, the IP address, and the host names gleaned from doing a reverse lookup on the IP address. 

15. While logged into a student machine, run the `ifconfig` command.  Find the MAC address for the machine that is used for Internet access (find the IP address you just looked up, find the hex address after `ether`).  

16. Create a file named `host-ip-mac.csv` where the first line contains the string `host,ip,mac`.  Add two lines with values from **two** of the student machines using a text editor of your choice.

   > Note that you will need to login in to those respective students machines.  `student10.cse.nd.edu` and `student11.cse.nd.edu` are two of the machines.     

17. On the student machine, using `ls -l`, determine the name of the group that is associated with all of your files. Re-direct the output of the command to a file named `out-ls`.

18. On the student machine, use the `id` command via `id --group --name` to list the group name that is associated with your username. Create a file named `mygroup` that contains the output from that same command. 

19. `touch` a file named `pythoninfo.txt`.  Append to the file the version of Python (e.g. `python3 --version`) that you are running by default.  Switch to the **virtual environment** used in Homework 3 or 4.  Append to that same file the version of Python that is invoked.  Append the result of the `ls -l` command from the `bin` directory in your Python virtual environment directory to the file.

20. Write a simple Python bit of code named `doErr.py` that has two print statements, one that will output correctly and one that will have an error. Create two files, one named `doErr-NoErr.txt` that redirects only `stdout` when running your file and another named `doErr-StdErr.txt` that redirects both `stdout` and `stderr` when running that code. Compare and contrast the two files. 

   > It is entirely up to you how to write a bit of buggy Python code.  An undefined variable might be a great starting point for the second print statement.  

## Submission

When you are done, switch back to the main branch via `git checkout main` and then merge in your branch via `git merge netid\hw05`.  Push your changes out to GitHub. 

> Don't forget to commit prior to doing your merge.  `git` will thankfully remind you.

For the Canvas side of the submission, submit the hash of your last git commit. Use `git log` or go to the Github website to get that hash and copy / paste it as the accompanying text for Canvas. Observe what appears in the log on Github (if anything) related to your branch and the respective commits. Do you see the branch at all? Why or why not?

## Rubric

14 points total

* 2 pts - No intervention required for the submission
* 1 pt - `README.md` present and contains the right information
* 1 pt - Canvas contains the right information (hash)
* 1 pt - Correct usage of branching
* 1 pt - Three or more commits are present
* 8 pts - All requested files present and correct
