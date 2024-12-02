# Homework 10 - Penultimation

In this homework, we will be bringing together multiple components that we have discussed in class.  Specifically, we will be doing a blend of Python and C programming whereby we have one process written in Python connected via the network via an approach called ZMQ (Zero Message Queuing) to our C code.  Furthermore, we will also add in requests from a remote web server for the data that will be processed by Python.  Everything will come full circle then with small wrapper scripts then to invoke the C code which in turn works with the Python server.  

| **Aspect** | **Information** |
| --- | --- |
| **Assigned** | Monday, November 25th |
| **Due** | Monday, December 9th - 10 PM |
| **Due (EC)** | Friday, December 6th - 10 PM |
| **Group** | Group |  
| **Canvas Link** | Homework 10 |
| **Points** | 100 |

## Task 1 - Install ZMQ

Take a look at [ZMQ (aka ZeroMQ)](https://zeromq.org).  Using what we learned in previous homework, install ZMQ to your virtual environment.

## Task 2 - Understanding the Assignment

Up until now, we have written various one off scripts but have yet to try to put things together into a bigger "system".  While we did get a bit bigger with a few of the homework assignments, this will be the first one where we intentionally try to blend all three approaches from class.  

Specifically, we are going to do the following:

* *Python:* We will use Python as our server that will respond to requests from the C client to extract various data points, filter that data, and then pass that back to the C client.  We will also bring back unit tests to help test the core functionality.  

* *C:* We will use our C client to make requests to the Python server reading from `stdin` either directly or through re-directed file.  

* *Scripting:* We will create various shell script that wrap various invocations of our C code to provide the "illusion" of command-line executables.  

### Unifying Different Languages

The challenge when we try to bring multiple sets of code together, particularly across languages, is how we connect the two code bases together.  For that purpose, we will be using a simplified messaging protocol that allows us to communicate between two processes using network sockets.  Whereas socket-based programming can be challenging, we will be using a particular package called [ZMQ - Zero Message Queuing](https://zeromq.org). ZMQ takes away many of the "hard" parts of network programming allowing us to easily send messages back and forth between processes.  It does so with multi-threading already built in allowing us to focus on how we send messages without worrying about waiting, blocking, or other odd network minutia.  

### The Bigger Picture

Much of the heavy lifting for our system will be done in Python.  We do this because Python has much nicer libraries to help out with processing JSON files and fetching data via web requests (HTTP).  

However, we cannot always work in our language of choice and sometimes need to handle various ugly / edge cases.  In that case, we are going be bringing the uglier side using C.

At a high level, we are going to be doing the following.  A user will run the C program.  This C program will take input from the user with regards to what queries to make with respect our set of JSON data.  Since C is not terribly great at parsing JSON, the C code will send the query request via ZMQ to the Python code.

Our Python code will be our "server" that will largely sit and wait for requests from our C client.  Upon receiving a request from the C program, the Python code will either directly answer the query and / or fetch new data from a remote web server.  Requests can include picking which dataset to use (e.g. 2024-01-21), asking for the information for a specific data instance.  While in class the original concept was going to be to do interactions with beacons, we are going to do a slight audible and keep working with the original speed test data from earlier homework.  

When the Python code resolves the specific request, it will send the data back in a C friendly format (e.g. CSV).  The C code will then display that data back to the user.

So what skills will we be learning on this assignment:

* ZMQ - We will be using ZMQ and network communications to cross between two different processes (one in Python, one in C)
* HTTP - We will be using Python to request data from a remote web server
* JSON - We will be doing more JSON processing for data that comes back from the web server
* CSV - We will once again be converting to a CSV from a JSON but doing this in a way to send from Python to C

### Python Code

As described in lecture, the Python code will launch with two arguments: the URL to fetch data from and the port number to use for the server.  Once the ZMQ server is launched, the Python could will do the following in a loop:

1. Wait for a request from the C client via ZMQ
2. Process the request from the C client that follows the form of:
   * `stat, date, time, filter` creating a new queue of data
      * `stat` can be one of `count`, `mean`, `median`, `min`, `max`, `stddev`, `list`
      * `date` is either `*` (any date) or a valid date in `YYYY-MM-DD` format with a wildcard allowed for any of the sub-fields (YYYY or MM or DD) but only for an entire sub-field, e.g. `20*` is not allowed but `*-05-15` is allowed
      * `time` is either `*` (any time) or a valid hour ranging from 0-23
      * `filter` conducts a selection of certain fields such as `iface` (interface), `dir` (direction), or `type` (type of test)
      * The filter is specified by writing `iface=eth0;dir=downlink;type=iperf` where the field name is stated and the value follows an equals sign.  The fields to filter on are separated by semicolons.  By default, the default filter should be `iface=eth0;dir=downlink,type=iperf`.
   * `list` is a special kind of stat command that lists the data one line at a time
      * When in `list` mode, the `success` result returns the number of matching records
      * Each individual record is then fetched using `more` 
   * `exit` finishing up the current session (for the client)
3. Send a reply to the C client based on the requested operation that is either `success` or `failure` followed by an explanation.  
   * When commands are successful, it should generally have `success` followed by the appropriate command response
      * For all commands that request a stat besides `list`, it should be the `stat` name and the value
      * For a `list` command, the first response should be the number of records that will follow
      * For a `more` command, the next line is sent starts with `success` if there are more records followed by the number of lines still to send and then all fields present following a pattern of fieldname, fieldvalue through each of the fields
   * When commands are unsuccessful, it should generally have `failure` followed by the reason for failure of the command
4. Repeat the loop

    {
        "timestamp": "2024-05-03T02:26:16-04:00",
        "mac": "DC-A6-32-1D-A4-E0",
        "test_uuid": "93f8e208-2425-4e9f-b4c7-17d77b1c211a",
        "type": "iperf",
        "direction": "downlink",
        "interface": "eth0",
        "host": "ns-mn1.cse.nd.edu:5212",
        "isp": "unknown",
        "duration_s": 5.000101,
        "transfered_mbytes": 495.1798,
        "tput_mbps": 792.2716761121426,
        "std_tput_mbps": 25.984215820781476,
        "max_tput_mbps": 814.9305375808235,
        "min_tput_mbps": 741.7217525614017,
        "median_tput_mbps": 799.9386373617318
    },


For instance, the server would display the following:

```
% python3 theServer.py support/grading/hw03/hw03-config.json 41003
Server started successfully - listening on Port 41003!
Waiting for a new command
RCVD: count, 2024-05-18, *, iface=eth0;dir=downlink,type=iperf
count requested - 35 records 
SENT: success, count, 35
Waiting for a new command
RCVD: mean, 2024-05-18, *, iface=eth0;dir=downlink,type=iperf
mean request - 35 records
SENT: success, average, 16.7382
RCVD: avg, 2024-05-18, *, iface=eth0;dir=downlink,type=iperf
error - bad command avg
SENT: failure, bad command - avg
Waiting for a new command
RCVD: list, 2024-05-18, 1, iface=eth0;dir=downlink,type=iperf
SENT: success, 2
Waiting for a new command
RCVD: more
SENT: success, 1, timestamp, 2024-05-03T02:26:16-04:00, mac, DC-A6-32-1D-A4-E0, ....
Waiting for a new command
RCVD: more
SENT: success, 0, timestamp, 2024-05-03T02:26:16-06:00, mac, DC-A6-32-1D-A4-E0, ....
Waiting for a new command
RCVD: more
SENT: failure, no more data to send
RCVD: exit
SENT: success, exiting
```

### C Code

The C code will do something fairly similar:

1. Wait for input from the user
2. Check that the input is valid and send it to the Python code via ZMQ
3. Wait for the response from the Python code
4. Display the result
5. Repeat the loop

The C code is much simpler than the Python code but you will have some freedom to add functionality of your choosing for the code.  

## Task 3 - Small Scale Testing - ZMQ

To start, copy over the contents of the `example` sub-directory Homework 10 to your private repository.  Remember to use `cp -R`.  

### Task 3a - Get It Working

There are two examples of code in the `example` directory, one that is a client in C and another that is a server in Python.  

You should do the following:

1. Pick a port to use for both of the pieces of code.  Right now, the code uses port 40000.  Offset the 40000 by an appropriate number, e.g. say by 2 or 3 digits from your 900 number.  Make sure to change both the Python code and the C code to use the same port number.
2. Build the C code via `make`
3. In one SSH session, run the Python server.
4. In a different SSH session on the same student machine, run the C client. 

Confirm that a familiar message (`GO`, `IRISH`) properly appears.  

## Task 4 - Make It Go

For Task 4, your task is then to make this all go using the information from Task 2 as your guide along with the following information:

* Put your code in a directory named `solution`.  In the `solution` directory, create two sub-directories, `server` for your Python code and `client` for your C code.
* Use a similar port number to the one that you picked for Task 3.  Make sure that the port number is the same for both the server as well as the client, e.g. the client connects to the port on the server.  

When you are finished, you should have the following directory setup:

```
# Your README.md goes here
hw/hw10/
hw/hw10/data/
hw/hw10/example/
hw/hw10/solution/
# Your Python server
hw/hw10/solution/server/
# Your C client (.c, Makefile, etc.) and various shell scripts
# that wrap around your C client
hw/hw10/solution/client/
```
### Task 4a - Server Behavior (Basic)

Your Python code should be named `theServer.py` and should be located in the `solution/server` directory.  It should take two arguments, the URL from which to fetch the data (the `data-all.json` from Homework 3 is a great starting point) and the port number to use for the server.

The server should loop endlessly until Control-C is pressed.  At that point, your server should catch the Control-C signal and gracefully exit.  You are welcome do display any debug information as you see fit.

The code for processing the JSON file should be contained in a file named `processdata.py`. That file should be imported by `theServer.py`.  You should write appropriate unit tests for `processdata.py` to confirm appropriate functionality. While it does not need to be comprehensive, it should test some basics of your code.  

### Task 4b - Client Behavior (Basic)

* The client should expect to read information via STDIN.  You can expect that `exit` will cause the client to gracefully exit / quit.
* The C client should take in the host name and port number as its first two arguments. 
   * ./client localhost 41003
* You may use whatever syntax you would like in terms of arguments for your C code including order after the first two arguments etc. 
* Write a script for your C code named `test.sh` that demonstrates key parts of functionality for your C code and document that in `README.md`.

### Task 4c - Shell Wrapper

Create a file named `.config` in your client directory that contains the following information:

```
EXE=client
HOST=localhost
PORT=41003
```
where HOST is the hostname (typically localhost) and PORT is the port that your server is running on.

Write the following shell scripts that read the `.config` file to launch your C code providing the following functionality:

* `bb` that operates in the following manner
   * If the first argument is not `-query` then the next arguments must be year, month, day, and hour
   * If the first argument is `-query` then the next argument must be a filter string (e.g. iface=eth0;...) followed by the year, month, day, and hour
   * This command may simply echo what is provided by the server in a raw manner
   * To support list, you may want to have a command that is only in your C code but executes a batch of commands, e.g. `listmore` that does a list and a more

The script should be appropriately executable.

At this point, the code as robustly written will get you to 85 percent of the potential score.

## Task 5 - Clean Output

While `bb` provided functionality in a raw format, the final task is to clean up the output in a script named `bbf`. Your task is then to improve the code and add in functionality that you feel appropriate to get to a higher score.  You may choose whatever you see is fit to get there or simply call it good at 85 percent through Task 4.

* `bbf` that operates in the following manner
   * Same requirements with respect to `-query` or without it as with `bb`
   * Adapt the output to be "fancier" and cleanly formatted.  It is up to you where this cleanup occurs whether it makes sense to put it in the C code or in a shell script  
   * It is up to you what a clean output looks like and how clean you make the output
      * For full credit, you should clean up the normal stat requests as well as list though you may reduce the number of fields reported in a list

The script should be appropriately executable.

## Submission

**REMEMBER:** We will only be testing your code on the CSE student machines.  If you choose to develop on your Mac or Windows machine, we will not attempt to replicate your same setup.  Your code will be graded as it runs on the student machine.  

The submission will be the same procedure as past homework.  That means:

* Commit early and often as part of the homework
* Push your changes when you are absolutely done
* Submit the hash of your final commit via Canvas
* In your `README.md`, feel free to add any additional commentary about what works or does not work with your code. 
* Your `README.md` should thoroughly document what if anything that you have done for Task 5.  

## Rubric

To be added after Thanksgiving break