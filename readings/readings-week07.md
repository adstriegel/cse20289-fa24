# Readings - Week 07 - CSE 20289 - Systems Programming

With the completion of the first exam, we are now transitioning into shell scripting and understanding the UNIX command line.  The readings below are fairly extensive and you are welcome to appropriately skim them.  

Remember that quizzes will resume this coming week.

## Readings

From [The Linux Command Line](https://linuxcommand.org/tlcl.php):

* Chapter 6 - Redirection
* Chapter 7 - Seeing the World as the Shell Sees It
* Chapter 8 - Advanced Keyboard Tricks
* Chapter 9 - Permissions
* Chapter 10 - Processes

You should also try converge on one command-line friendly editor that becomes your go to editor of choice:

* [nano](https://www.nano-editor.org): This text editor is the easiest to get started with, but also has the least amount of features. 

   * [Using Nano](https://www.linode.com/docs/tools-reference/tools/using-nano)
   * [The Beginner's Guide to Nano](http://www.howtogeek.com/howto/42980/the-beginners-guide-to-nano-the-linux-command-line-text-editor/)

* [vim](http://www.vim.org): This text editor has many features such as syntax highlighting, plugins, and even spell checking. That said, it has a steep learning curve due to its foreign keyboard interface.

   * Chapter 12 - A Gentle Introduction to Vi of [The Linux Command Line](https://linuxcommand.org/tlcl.php)
   * [Vim Introduction and Tutorial](https://blog.interlinked.org/tutorials/vim_tutorial.html)
   * [Interactive Vim Tutorial](http://www.openvim.com/tutorial.html)

* [emacs](https://www.gnu.org/software/emacs/): This text editor also has many features (perhaps too many). 

   * [Absolute Beginner's Guide to Emacs](http://www.jesshamrick.com/2012/09/10/absolute-beginners-guide-to-emacs/)
   * [Modern Emacs](https://www.modernemacs.com/)

It is entirely up to you as to your choice of editor.  There are a legion of posts and fights about which is the one true editor ([vim versus emacs](https://en.wikipedia.org/wiki/Editor_war)).  As emacs tends to be a bit heavier weight, generally knowing enough vim is a solid choice as it tends to be present on most systems.

While we want to be reasonably knowledgeable for the class about command-line based editors, it is also entirely OK to embrace excellent IDEs.  Visual Studio Code is amazing and highly recommended.  

### Questions - Looking Ahead 

* What are the three special files available to a program when it is executed?
* How would you do the following types of redirection:
   * Redirect the contents of a file into a command
   * Redirect the output of a command into a file
   * Append the output of a command to a file
   * Redirect only the error messages
   * Redirect both the error and output
   * Redirect the output of a command into another command
* What is brace expansion? What is pathname expansion?
* What are environmental variables for the shell?
* What is an example of an environmental variable?
* How do we set the value of a variable in bash?
* When we list files via `ls -l`, what do the various outputs mean?
   * Where is the user information?
   * Where is the group information?
   * What does other mean?
* What does it mean to / how do you:
   * Suspend a running command
   * Resume a previously suspended command
   * Indicate the end of input
   * Terminate the process via keystrokes
   * Terminate a process if we know its PID            