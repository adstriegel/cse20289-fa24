# Recap - Lecture 23 - Topics Covered

In today's lecture, we focused on pipelining, diving deeper into several examples of pipelining, specifically code that extracted the top users from the list of running users. 

## Relevant Links

* Slides - See also Canvas
   * [PDF - 1 Page per Slide](https://canvas.nd.edu/files/4177270/download?download_frd=1)
   * [PDF - Notes Format](https://canvas.nd.edu/files/4177269/download?download_frd=1)
* [Panopto Lecture Recording](https://notredame.hosted.panopto.com/Panopto/Pages/Viewer.aspx?id=09849a7d-db62-4005-837e-b217011208b2)

## Clarifications

* What does `grep -v` do?
   * `grep -v` prints any line that does not match the criterion.  The `-v` is for inverse (yeah, not exactly the most straightforward given how `-v` is usually verbose).
* What is the range for `cut`?
   * The field count for `cut` starts at 1, not 0.  

## Key Points

* What is pipelining? What does the `|` connect between processes?
* What does `grep -E` do? How is that different from `egrep`?
* What do commands such as `cut`, `tr`, and others do?
* How does the UNIX philosophy relate to pipelining?
   * Have a command that takes in text and serves one purpose extremely well
* Be able to read / interpret command such as those listed in the [pipeline.sh](./pipeline.sh) example.
* What is the purpose of `trap`?

## Looking Ahead

Homework 6 currently looks at writing unit tests and then using a shell script to interpret the results.  You are welcome to look ahead to Homework 7 where you will try to write a shell script that automatically unpacks and archive and examines it for various strings / content.

