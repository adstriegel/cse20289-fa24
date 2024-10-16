# Recap - Lecture 21 - Topics Covered

In today's lecture, we continued to look at several shell scripts as well as oddities associated with shell scripts.  We looked at a revised `isitdown.sh` script and examined the foundation for pipelining, namely how we can read from `STDIN` or from a file.  We also looked at the oddities of read and why / when a newline (or lack thereof) can create issues.  

## Relevant Links

* Slides - See also Canvas
   * [PDF - 1 Page per Slide](https://canvas.nd.edu/files/4148077/download?download_frd=1)
   * [PDF - Notes Format](https://canvas.nd.edu/files/4148076/download?download_frd=1)
* [Panopto Lecture Recording](https://notredame.hosted.panopto.com/Panopto/Pages/Viewer.aspx?id=9acc4ecf-960f-4774-b150-b20b010db818)

## Key Points

* Be able to read and parse the [`isitdown.sh`](./isitdown.sh) script
* What did the `{1:-/dev/stdin}` code do?
* Why did we add the `||` (double pipe) in the code to make it more robust?
* What was the purpose and main components used when we started to sketch the solution to `findBig.sh`?
* What is a bracket expansion that was mentioned to help with `findBig.sh`? See [tricks.sh](./tricks.sh) 

## Looking Ahead

In the next lecture, we will look at a solution to `findBig.sh` as described in class.  We will continue by doing a deeper look into pipelining and thinking about how filtering and pattern matching (specifically regular expressions) will help us in corraling and steering the respective text flows between our respective shell commands. We will also explore aspects such as functions, scope, and various other nuances of shell scripting. 
