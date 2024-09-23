# Homework 3 - CSE20289 - Fall 2024 - DRAFT

In this homework, we are going to bring along several aspects from class including virtual environments, JSONs, web requests, as well as two new aspects, creating plots via matplotlib and editing Word docs via docx. 

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, September 23rd |
| **Due** | Monday, September 30th - 10 PM |
| **Due (EC)** | Friday, September 27th - 10 PM |
| **Group** | May be done as a group |  
| **Canvas Link** | Homework 03 |
| **Points** | TBA |

## Task 1 - Setting up the Environment

### Task 1a - Virtual Environment

In order to be able to use packages that are not installed by default on the student machines, we will be taking advantage of Python [virtual environments](https://docs.python.org/3/library/venv.html).  This will allow us to install any packages as we see fit that will end up being accessible to your Python code.

```
% python -m venv ~/cse20289-fa24
% cd ~/cse20289-fa24
% source bin/activate
```

The first command creates a Python virtual environment while the `source bin/activate` command invokes the virtual environment.  After running the source command, you should see your prompt note the virtual environment being used by putting the name of the virtual environment `(cse20289-fa24)` in parentheses.  You can exit the virtual environment at any time by running the `deactivate` command.  

> **Note**: Each you login to the student machine, you will need to run `source ~/cse20289-fa24/bin/activate` in order to start up your virtual environment.  

```
% which python3
~/cse20289-fa24/bin/python3
```

You should see that the `python3` actually is found now in your home directory.

Now that we are in the right virtual environment, we will want to install the needed packages for the homework.  Note that you now have complete control when operating in this environment to install the Python packages as you see fit.  

Install the following packages:

```
% python3 -m pip install python-docx
% python3 -m pip install numpy
% python3 -m pip install matplotlib
```

### Task 1b - Confirm Installation

While staying in your virtual environment, do a fresh pull of the class repository (in your repos directory).  Confirm that you can execute the two example files in the `hw/hw03/ex-src` directory without errors:

```
% python3 ~/repos/cse20289-fa24/hw/hw03/ex-src/plot-graph.py
% python3 ~/repos/cse20289-fa24/hw/hw03/ex-src/create-word.py
% ls
```

Using either `scp` or an application such as [FileZilla](https://filezilla-project.org), download the Word document as well as the created PNG.  Open up the image as well as the Word doc to confirm that they were created correctly.  

### Task 1c - Share Group Access

For this homework, we will be using the group repository that you created in the first assignment (ex. `group-cse20289-fa24-striegel`). If you are in a group, select one group member to host the repository.  Make sure if you are in a group to extend collaborator access to each of your group members. If you are working individually, you will not need to extend collaborative access but you should use that same group repository location.

Create an appropriate `hw/hw03` directory for where to place your source code for the homework.  Create a `README.md` file and include the names of the group members as well as the e-mails of any group members.  Do an initial commit of your repository.

### Task 1d - Create the Files

For the homework, you will be creating three files:

* `checktests.py` : This is the main code which is responsible for downloading the CSV from the website, parsing the JSON, and then properly invoking the other two source files.
* `plotdata.py` : This code will take in a set of data and create a graph containing the results.
* `createreport.py` : This code will create a Word document summarizing the data on a month by month basis.  

Briefly review how to [invoke functions from different Python files](https://www.geeksforgeeks.org/python-call-function-from-another-file/).  Unlike in the last homework where we used `subprocess`, we will be intentionally doing an `import` to bring in code from multiple files.  

## Task 2 - Understanding the Assignment

The data that you will working with was gathered from a project studying [broadband speeds in underserved areas](https://wireless.nd.edu/research/new-methods-for-residential-broadband-monitoring/).  In particular, you will be working with two datasets, one gathered directly from Prof. Striegel's house and the other gathered from Prof. Ghosh's house.  

Essentially, there is a Rasbperry Pi installed at each home with the Raspberry Pi configured to run an [`iperf`](https://iperf.fr) every hour to examine the broadband performance receieved, both across the wired (Ethernet - `eth0`) interface as well as the wireless (WiFi - `wlan0`) interface.  Your task is to write code that will fetch the dataset as a JSON from the web server, filter the data for the relevant components, and then to create a report embedded in a Word document where the report contains various statistical as well as graphed summaries from the dataset.

There are several examples of the data placed in the `ex-data` sub-directory in the class repository for `hw03`.  For the purposes of this effort, the relevant fields are as follows:

* `timestamp`: The time at which the test was run.  Note that the data is not sorted by time in the JSON.  
* `interface`: The interface on which the test was run, generally having one of two values: `eth0` for a wired (Ethernet) test and `wlan0` for the WiFi interface.  
* `type`: The type of test.  For this homework, we will want to only consider `iperf` tests.
* `direction`: There are two different values, `downlink` and `uplink`.  For the purpose of this assignment, we will focus on only the downlink.
* `tput_mbps`: The measured throughput (speed) of the link in megabits per second.  

For the purposes of this homework, we are interested in creating a report to be shared with participants in the study that creates a Word document containing monthly statistical summaries as well as plots of the day to day average performance as a set of bar graphs. 

To accomplish this task, we will be using three different packages: `numpy` for the statistics, `python-docx` for the Word document, and `matplotlib` for the graphs. You should have all three of these packages installed using the Python virtual environment as described in Task 1.

The overall workflow will be as follows:

1. Download the JSON file from the URL specified in the argument
2. Sort the data according to the timestamps
3. Filter out the data appropriately
4. Compute and display the statistical summaries (number of points, mean, median, standard deviation, inter-quartile range) for the overall data.  Do this breakdown amongst for each interface, e.g. wired (Ethernet) and wireless (WiFi).  
5. Compute and display the statistical summaries on a month by month basis
6. If requested, create graphs via `matplotlib` for each of the months
7. If requested, create a Word document and include graphs if generated.

## Tasks 3-5

Tasks 3 through 5 may be done in parallel by different group members.    


## Task 3 - Fetch and Filter the Data

Write code in `checktests.py` that takes in a single required argument which is the URL to fetch for the JSON file.  The following JSONs are available to download:

* `http://ns-mn1.cse.nd.edu/cse20289-fa24/hw3/data-10.json` - This is a small JSON consisting of only ten data points.  You can find example code that reads a direct file in the [parse-json.py](./ex-src/parse.json) file.  The data file itself can be found in the `ex-src` directory.  

* `http://ns-mn1.cse.nd.edu/cse20289-fa24/hw3/data-250.json` - This is a medium JSON that contains 250 data points from the study.  

* `http://ns-mn1.cse.nd.edu/cse20289-fa24/hw3/data-1000.json` - This is a medium JSON that contains 1000 data points from the study.  

* `http://ns-mn1.cse.nd.edu/cse20289-fa24/hw3/data-all.json` - This is a full JSON containing all of the data as recorded at Prof. Striegel's house across the entirety of the study in beautified (indented format).  This should be the last file that you try to operate on. 

* `http://ns-mn1.cse.nd.edu/cse20289-fa24/hw3/data-raw.json` - This is a full JSON containing all of the data as recorded at Prof. Striegel's house across the entirety of the study.  This should be the last file that you try to operate on.  It is the same as the `data-all.json` dataset but is not formatted to make it easier to read.  

Write code to fetch the data from the server and confirm that you are able to fetch and display the data for the smallest dataset instance.  

### Design Choice - Sort or Filter First?

Once you have confirmed that you can download the data, look at filtering the data to include only what we want (`downlink`, `iperf`) and to appropriately sort the data according to its timestamp.  You could elect to do the filter or sort first but think through why you might want to do one first versus the other.  State the group's choice and rationale in the `README.md`.  

Implement your choice through a function that takes in a received dataset and then gives only the relevant, sorted data as needed. 

### Filter the Data

Write a function that returns a filtered subset of data that takes in: the dataset, a month (the default should be 5), a year (default should be 2024), and an interface (default should be `eth0`). The function should return a list of all data points that satisfy the requested filtering.   

### Analyze the Data

Write a function that takes in a list of data points, the requested interface to examine (`eth0` or `wlan0`) and returns a populated dictionary containing the statistics for that dataset (see below).  

* `Period`: The period over which the data was generated.
* `Interface`: The interface on which the data was generated (wired or wired).
* `Num Points`: The number of the data points present
* `Min`: The minimum throughput observed across the data points
* `Max`: The maximum throughput observed across the data points
* `Mean`: The mean throughput across the data points
* `Median`: The median throughput across the data points
* `Std Dev`: The standard deviation for throughput across the data points  
* `10th Percentile`: The 10th percentile for throughput across the data points
* `90th Percentile`: The 90th percentile for throughput across the data points

> *Hint*: To compute the 10th and 90th percentile, look at the quantiles function offered by the `statistics` package.   

### Output the Data

Write a function that takes in that generated dictionary (field, value) and outputs that dictionary information to a string as follows:

```
Period: All
Interface: Wired
Num Points: 2
Min: 0
Max: 0
Mean: 0
Median: 0
Std Dev: 0
10th Percentile: 0
90th Percentile: 0
```

The `interface` should be either `Wired` (Ethernet) or `Wireless` (WiFi). The period is either `All` or the month and the year together (`2024-05`).  

### Bring it Together - Task 3

Bring all of your code together to do the following:

* The only argument to your code to start will be the input URL
* Start by fetching either the 10 or 250 point dataset from the web server
* Compute the statistics across the relevant, sorted dataset for all data points for both the WiFi (`wlan0`) and the Wired (`eth0`) interfaces.
* Output the statistics to the console
* Fetch the 1000 point dataset and check your results
* Filter the 1000 point dataset for only May 2024
* Fetch the full dataset
* Confirm operation for both of the full datasets for the full data as well as only the May 2024 subset for each 

Reference output will be provided for the smaller datasets at mid-week.

## Task 4 - Plotting

For Task 4, write code in `plotdata.py`.  For testing purposes, insert the following code:

```
if __name__ == "__main__":
```

This block of code will let you identify when this particular Python code is being invoked directly, e.g. `python3 plotdata.py` as opposed to being invoked through an import of the functions from the code.  You should use `argparse` to take in three arguments when invoked directly, the name of to read in as a JSON, the number of days in the month, and the name of the file to output. 

Place the two files `data-2024-05-eth0.json` and `data-2024-05-wlan0.json` into the `ex-data` directory in your repository for testing purposes.  You may find those files in the class repository.

Look at the [plot-graph.py](./ex-src/plot-graph.py) for an example of creating a Python bar graph plot. You may either build on this code or start your own code for the purposes of this effort.

The goal of the code is to provide the following functionality:

* Take in an input dataset and the targeted filename for the graph
* Compute the average daily throughput across that particular month
* Create a graph from the first day until the last day of the month
* Output a PNG using the specified filename

### Create the Testing Framework

Write the code to properly capture the arguments when invoked directly as a Python script including capturing the input file, the number of days in the month, and the output filename.  

> *Hint:* Look at the [split-json.py](./ex-src/split-json.py) for examples of using `argparse` for arguments.  

### Average Daily Performance

Write a function that takes in an input dataset and the number of days in the month and then computes the daily average.  You may assume that the dataset has been pre-filtered to only include only WiFi or Ethernet as well as already being filtered to have only days from a given month.  If a day has no data, the average should be marked as zero.  

Your function should return an appropriate data structure that denotes that day and the average for that particular day.  

### Create a Plot

Write a function that takes in the data structure that you recently created and an output filename to create a bar graph plot where each day and its average represents an individual bar in the graph plot.  The Y axis label should be `Average Throughput (Mb/s)` and the X axis label should be `Day` where each day should be output from 1 until the last day in the month.  

Output the file as the specified filename. 

### Test Your Code

Test your code on the two input datasets. If you are not running your code on your local machine, you will need to download the output PNG files and confirm the correct output.  

Reference output data examples will be provided at mid-week.

## Task 5 - Word Document

For this task, your code will be located in `createreport.py`.  Use a similar approach for testing to Task 4 where testing will be conducted when the code is invoked directly.

For Task 5, your code should take in three arguments:

* A text file to read in
* An image (PNG) to read in
* A requested output file

### Write the Word Document

Using the [create-word.py](./ex-src/create-word.py) as guidance as well as the `python-docx` package, write code that reads in a text file and creates an output Word document.  

### Create a Table

Read about [table creation](https://python-docx.readthedocs.io/en/latest/api/table.html) in the `python-docx` documentation.  Drawing upon your data structure from Task 3, create a dummy instance of the dictionary entry holding statistics (all entries are zeroes). Using that dummy instance, write a function that takes in that dictionary entry and a docx object to insert a well-formatted table.

### Combine All Parts

Write a function that takes in four parameters: a text string, the statistic dictionary, the PNG for the graph, and the output filename. Modify your code to invoke this new function when executed directly as a script where the text string is drawn from the text file (appears first in the Word document), the table containing the statistics values, and the input PNG. 

## Task 6 - Unify Your Code

Modify your `checktests.py` in the following manner:

* The first argument should be the year
* The second argument should be the month
* The third argument is for a text file to put at the start of the file
* The fourth argument should be the URL from where to fetch the data

The `checktests.py` code should import the necessary functionality from the other two source code files that you have written.  

Your overall process should fetch the data from the specified URL, filter / sort the relevant data for that particular year and month, and then output two reports, one for the Wired side named `Year-Month-Wired.docx` and one named WiFi named `Year-Month-WiFi.docx` where *Year* and *Month* are the actual year (2024) and month (05) values. Each of the output Word documents should include the initial text file content, the statistical summary for the data in a well-formatted table, and the plot for the month of data.  

> *Note:* You are responsible for figuring out how to manage the various days within a given month as it relates to your code. 

Your code should also avoid leaving extra files, making sure to clean up any intermediate files that were created (e.g. the PNGs).

## Task 7 - Robustify Your Code

Finally, think about your code could break as an part of the processes and add in appropriate hardening / handling of errors that could occur.  For instance:

* What happens if the URL does not fetch anything? Valid / good JSON?
* What if the year / month request has no data after filtering?
* Could any of the stastical functions fail with insufficient data?
* What should happen if the Word file already exists? 

Protect your code to the best of your ability such that it fails gracefully and appropriately educates the user in the event of bad input.

## Submission Checklist

Before you submit your code, make sure to check the following:

* You have a `.gitignore` file to ensure that no Python caching files are included in your repository
* You have a `README.md` file along with all group member names and e-mails as well as any specific quirks for your code.  
* You have three pieces of source code present: `checktests.py`, `plotdata.py.py`, `createreport.py` where each code can be run individually
* You have multiple commit messages that have `hw03` at the start and if you have multiple group members, at least one commit per group member

**Push your working code** to the group repository and **submit the final commit hash via Canvas**. Only the owning group member of the repository needs to submit the hash. 

> **Note:** For the purposes of homework tracking, the GitHub date will be used for determining the core lateness penalty (e.g. the 25% per day).  The hash may be submitted late to Canvas but will result in a 5% reduction to the maximum score and the homework no longer being eligible for the extra credit bonus.  

## Extra Credit - Up to 15 pts

If the `checktests.py` code is invoked with a `--all` argument, ignore the month and year arguments and create two unified Word files (same concept of one per adapter) but enumerates across all present year and month combinations (e.g. 2024-05, 2024-06) in the dataset. Rather than being named `Year-Month-Wired.docx`, the file would be named `All-Wired.docx`.

## Rubric

To be added mid-week