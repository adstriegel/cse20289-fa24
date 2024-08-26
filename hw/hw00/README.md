# Homework 0 - CSE20289 - Fall 2024

The focus of this document is to get you set up for homework submission for CSE 20289 - Systems Programming for Fall 2024.  

| **Aspect** | **Information** |
| --- | --- | 
| **Assigned** | Wednesday, August 28th | 
| **Due** | Monday, September 2nd - 10 PM | 
| **Due (EC)** | Friday, August 30th - 10 PM |
| **Group** | To be done individually |  
| **Canvas Link** | Homework 00 | 
| **Points** | TBD | 

## Pre-Requisites

* GitHub account
* Git installed on your machine or use the student machines
	* Installed / accessible via a command prompt
    * [Git Download](https://git-scm.com/downloads)
* Browse and review the [About Git](https://git-scm.com/about) page

## Helpful Software

* [Visual Studio Code](https://code.visualstudio.com)
   * [Git in Visual Studio Code](https://code.visualstudio.com/docs/sourcecontrol/overview)
* [GitHub Desktop](https://desktop.github.com/download/)
* Shell Interfacing
   * Mac: Use the default ssh client via Terminal
   * Windows: [PuTTY](https://www.putty.org)
* File Transfers
   * [FileZilla](https://filezilla-project.org)

## Task 0: Make a Repository Home

On both the CSE students machines as well as your personal laptop, you will need to identify a location to store the various repositories.  Each repository is its own directory as well as various sub-directories where the content is located.  As this class will have multiple repositories (class repository that is read only, personal private repository for your individual assignments, group private repository for group assignments), one location where the repositories can be found will be ideal.  

Figure out where to place your repositories on your machine.  For instance, the examples in this homework are placed in the `~/Documents/repos/` on Prof. Striegel's Mac notebook where the ~ (tilde) represents the user of the machine.  For Windows users, somewhere in My Documents might be ideal.

For the CSE student machines, use your home directory and create a `repos` sub-directory for placing all of the respective repositories.  

## Task 1: Clone the Class Repository

For the first official interaction with `git`, we will [clone](https://git-scm.com/book/en/v2/Git-Basics-Getting-a-Git-Repository) the public class repository.  Cloning will create a local copy that we can synchronize (`fetch`) updates from the class repository.  The class repository is _public_ meaning that anyone can clone it which is what you will do in this step.

You will have read access (`clone`, `fetch`) to the repository just as any member of the public.  The class repository can be found at the following location:

[https://github.com/adstriegel/cse20289-fa24.git](https://github.com/adstriegel/cse20289-fa24.git)

You can browse to this location using a web browser and it is the URL that you will provide to `git` for cloning the repository. We will be cloning the class repository both to your local machine as well as on the ND CSE student machines.   

> Note that for this homework that you should do this all using command line tools.  You are welcome to use GUI-based tools in the class but you will be expected to understand how to use `git` and other tools also via the command prompt.  For Windows, use the `Command` prompt and for Mac, use the `Terminal`.   

Switch into that particular directory and while in the your repository holding directory, go ahead and clone the repository:

    % cd ~/Documents/repos
    % pwd
    /Users/striegel/Documents/repos
    % git clone https://github.com/adstriegel/cse20289-fa24.git

If you look now in your repository directory, you should now have the repository for the class in the `cse20289-fa24` directory. 

    % ls
    adstriegel.github.io	advos_book   cse20289-fa24		opsys-sp22
    %
    
Go ahead and change into the repository.
 
    % cd cse20289-fa24
    % ls
    README.md LICENSE homework

From there, you should see the various files that would be visible on the repository website (e.g. browsing to [https://github.com/adstriegel/cse20289-fa24.git](https://github.com/adstriegel/cse20289-fa24.git)).  If you list the files in the directory, you should see all of the respective files as well as the files contained within the various sub-directories.  

The actual magic itself for `git` is in a hidden directory, namely the `.git` directory.  Try changing into that particular directory and looking at the files:

    % cd .git
    % pwd
    /Users/striegel/Documents/repos/cse20289-fa24/.git
    % ls
    FETCH_HEAD  config	 hooks  info  object  refs
    HEAD  description  index	logs	packed-refs
    
We could dive into these files but for now, that is getting a bit weedy.  For now, just know that there is a `.git` directory that contains all of the actual repository data.  The files that you see are all of the ones that actually matter.

If at any time we would like to make sure we are up to date, we simply do a `pull` inside of the repository.  

    % pwd
    /Users/striegel/Documents/repos/cse20289-fa24
    % git pull
    Already up to date.
    
The `git pull` command looks inside the `.git` subdirectory to figure out where the remote repository is and then attempts to pull down any updates from that remote repository.  Since we just cloned the repository, there should not be any updates.  The act of doing a `clone` or `pull` are read-only commands and as such do not require any credentials if the repository is *public*. When we work with your private repository a bit later, we will need proper login credentials.  

> Should we `fetch` or should we `pull`, what is the [difference](https://www.geeksforgeeks.org/git-difference-between-git-fetch-and-git-pull/)? The answer is it depends.  `pull` tries basically bring you up to date and to fast-forward merge. That usually is OK but not always OK if lots of contributors are changing lots of things.  For our class, if you are the only one changing the repo, it tends to work just fine.  `fetch` takes a kinder, gentler approach. Think of `pull` = `fetch` + `merge`. 

### Summary - Task 1

At this point, we have now successfully cloned the class repository.  You should periodically make sure to pull (e.g. sync) to make sure that everything is up to date.  You will not be directly editing files in the class repository, only using those files for copying out and / or starting a new homework.

### Checklist - Task 1

You should have cloned the class repository in two locations:

* The CSE student machines in your home directory in an appropriate sub-directory
* On your local machine

### Bonus - Task 1

You can find this very file in the repository as well.  Browse to the `assignments\baseline` sub-directory and look for the `README.md` file.  Feel free to inspect how many times this particular file has been modified.

### Second Bonus - Task 1

You can also open up the class repository (or later your private repository) in any of the various GUI tools such as Visual Studio or GitHub desktop.  

## Task 2 - Passwordless Login

For your homework, we will be creating a separate, private repository.  While the class repository is public, your repository will be **private**, shared only with the grading TA (Annapurna Puttaswamy) and the instructor. 

> Although GitHub offers the ability to fork a repository (e.g. make a separate / new repository), the availability of a forked repository must be the same as the source repository. For the public class repository, that would mean that any forked repository by you the student would also be public, not exactly what we would like.  There are organizational controls / mechanisms that allow that functionality to change but we are going to try to operate with a largely vanilla usage of GitHub.

Since creation and pushing require write access, we will first need to set up a passwordless login for GitHub.  Previously, GitHub allowed both password-based and passwordless (keyed) logins.  Since then, passwords have been disabled.  Although we could lean heavy on various GUIs and their usage of authorization tokens (e.g. GitHub Desktop), it does behoove us to learn exactly how a passwordless login would work.  

We will be setting up `git` via SSH using pre-shared keys.  An overview of how this works with GitHub can be [here](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/about-ssh). This task will best be done on the CSE student machines.  

1. Login to the CSE student machines.

2. Generate SSH keys if you don't have them yet:

        # Accept the defaults, don't make a password if you 
        # want to go passwordless
        % ssh-keygen

   > The result of this step will create a key for usage inside of the `.ssh` directory in your home directory on the CSE student machine.  

3. Copy the contents of `~/.ssh/id_rsa.pub` to the SSH Keys section of your GitHub settings page:

    [https://github.com/settings/keys](https://github.com/settings/keys)
    
        # Copy and paste the contents of this file into GitHub
        % cat ~/.ssh/id_rsa.pub

    > The id_rsa.pub file is the public key half.  The private key is also present can be found in the `.ssh` directory as well.  You should **never** share the private side of the key (hence why it is called the private side).  You can also retrieve this file and bring it down via either `scp` (secure copy) or using FileZilla by looking in to the `.ssh` directory of your home directory on the CSE student machines.  

4. Edit/create `~/.ssh/config` to use this key with GitHub:

        # Add the following to your config (replace $NETID 
        with your netid)
        % vim ~/.ssh/config
        Host github.com
            User git
            Hostname github.com
            PreferredAuthentications publickey
            IdentityFile /escnfs/home/$NETID/.ssh/id_rsa

    Once this is done, you should be able to do `git` operations without a password. You will need to accept the host key the first time by typing in "yes".  

    > To edit the file, you can use either [vi](https://www.cs.colostate.edu/helpdocs/vi.html) or [emacs](https://www.gnu.org/software/emacs/tour/index.html).

### Task 2 - Summary

So what exactly did we just do?  We generated a [RSA key pair](https://en.wikipedia.org/wiki/RSA_(cryptosystem)). RSA works through extremely large prime numbers but effectively has two parts, a public key that you share with everyone and a private key that you keep secret / known only to you. RSA is what is known as an asymmetric key system whereby the public key can be used to decrypt messages that were encrypyted by the private key and vice versa.

We can see this in practice with the steps that we did above. The public half of the key is shared with GitHub.  That allows GitHub to encrypt a message that only you can decrypt if you have the private key (confidentiality). It also allows you to send a message to GitHub to prove that you are who you say you are (authentication, e.g. a message will decrypt via the public key if it was encrypted with the private key). Since we keep our private key safe, we can thereby prove our identity of our account to GitHub since only we should have the private key.  

### Task 2 - Checklist

## Task 3 - Creating a repository

For Git, there are two ways to create a repository: initializing a repository and then associating it with a remote repository or creating the remote repository and then cloning it locally.  Generally, using the web for creation tends to be a bit easier.

We will be creating two repositories eventually, one for you individually and one for group work but we will focus on the individual repository first.  

1. Login to your GitHub account and create a new repository named `student-cse20289-fa24-NETID` where `NETID` is your Notre Dame net ID (e.g. `striegel`). It is critical that the format is exactly as noted above to allow the various test scripts to operate correctly.  
2. Go to that repository and make sure that the repository is set as private.
3. Add Professor Striegel and the grading TA (adstriegel, aputtasw) as collaborators for your repository.  We will strictly be read-only but we do need access for your private repository.  

Now that we have it created, let's clone your repository then locally.  You should replace `adstriegel` with your GitHub username and the repository name to use your netid. This should be done on the CSE student machine to make sure that everything is working correctly.    

    % cd ~/repos
    % pwd
    /escnfs/home/striegel/repos
    # Example of cloning GitHub assignments repository to 
    # local or student machine
    % git clone git@github.com:adstriegel/student-cse20289-fa24-striegel.git
    remote: Enumerating objects: 3, done.
    remote: Counting objects: 100% (3/3), done.
    remote: Compressing objects: 100% (2/2), done.
    remote: Total 3 (delta 0), reused 0 (delta 0), pack-reused 0
    Receiving objects: 100% (3/3), done.
    % ls
    README.md
    %
    
> We need the username and hostname `git@github.com` to get access to your private repository together with the keys that were set up earlier.

If all goes well, your private repository will have been cloned appropriately (just like the class repository).  

At this point, you should now have two directories underneath your repositories directory, one directory containing the public class repository and the other one containing your private repository.  The class repository should have files while your private repository should be empty outside of a `README.md`.  

Repeat this step to create a group repository using the naming convention of `group-cse20289-fa24-striegel`. Confirm that your GitHub authentication works correctly by also cloning the repository that you will use for group work.  

Finally, complete the following [Google Form](https://forms.gle/yC8sxB7ZEtWpSMMn6) to share / confirm your repository locations.  

### Checklist - Task 3

You should completed the following:

* Cloned your private repository on the CSE student machine
* Cloned your group repository on the CSE student machine
* Filled out the [Google Form](https://forms.gle/yC8sxB7ZEtWpSMMn6)

### Bonus - Task 3

Using either the command line (ideally) or one of the GUI tools, clone your private and group repositories on your machine.  

## Task 4 - Practice the Homework Cadence

For the purposes of this class, you will be the only one editing the code in your private repository.  This makes things a bit easier as we do not have to deal with the ugliness of merging whenever someone makes changes to other files or the same file you were modifying.  We will talk about that during class and will get some practice having multiple code writers during later group assignments using the group repository.

> One way to also make edits is to use branching.  We will use branching later in the course and will be using frequent commits to start for the class.   

Our normal cadence will be something like the following:

1. Pull the class repository

        % cd ~/repos
        % cd cse20289-fa24
        % pwd
        /Users/striegel/Documents/repos/cse20289-fa24
        % git pull
        Already up to date.
        %
2. Switch to the directory with your repository

        % cd ..
        % cd student-cse20289-fa24-striegel
        % git status
        On branch main
        Your branch is up to date with 'origin/main'.

        nothing to commit, working tree clean
        %

3. Create a directory for holding this homework inside of the `hw` directory.  For this homework, we will use the same naming convention.

       % mkdir hw
       % cd hw
       % mkdir hw00
       % ls
       . ..

4. Copy over the files from the `mod` directory in the class repository over to the `hw00` directory.

       % pwd
       /Users/striegel/Documents/repos/student-cse20289-fa24-striegel
       % cp ~/repos/cse20289-fa24/hw/hw00/mod/* .
       % ls
       gitinfo.txt hello.py

5. Now that we have a set of files, let's add them all for tracking to `git`.

        % pwd
        /Users/striegel/Documents/repos/student-cse20289-fa24-striegel
        % git add --all 

    This adds all of the initial files to be tracked by `git`.

    > You will see the `pwd` command here and there.  This is to *print the working directory* to confirm that you are in the right location.
    
7. Commit the initial versions of the files.

        % git commit -m "HW00 - Initial commit"

    > Whenever we commit a file (or files), you should include the identifier for the assignment at the start of the commit, e.g. HW00.

    We have now added and committed the initial versions of the files to our local repository.  

    > The action of adding allows `git` to track changes to the files.  The action of
    `commit' saves a version (think draft) in the local repository.  

8. We can see the log for the current repository by running `git log`

        % git log

9. We can also check up on the status of where we are at relatively speaking on our local repository versus the remote repository (the one at GitHub).

        % git status

10. Take a look at what is up on your remote repository (the one at GitHub) by browsing to GitHub in your web browser.  Go to your private repository.  Note that your repository is still identical to when you created it.  
    
11. Let's change that by pushing our content locally to the server.

        % git push

12. Now, go back to your repository on the GitHub website.  See if the files that we just added are now present.  You should see that there is a `hw` directory as well as the `hw00` sub-directory and the two files that you recently added.  You can also browse the history (look for the clock icon and click on it).  

13. Create and add a single file in your repository.  

        % cd hw/hw00
        % echo "Clever message goes here" > clever.txt
        
14. Add that new file and commit it.  

    Think about what you want to add and commit (clever.txt).  

        % git add clever.txt
        % git commit -m 'HW00 - Added clever.txt'
    
15. Modify the two files that are already present.  `gitinfo.txt` should be modified from Prof. Striegel's GitHub account name to your own GitHub account name.  `hello.py` should be modified to have your NetID present instead of Prof. Striegel's ND NetID.  For each of the modifications, make sure to do a `git commit` each time following the same patterm (start with `HW00`, descriptive name for what was done).  

   > Generally, we will want to try to do commits periodically as you work on your code.  It allows you to potentially undo changes or at least have a way to see how your code has evolved.  Remember, it is only when you `push` when the overall code gets sent to the remote repository at GitHub.  

16. Push all of your respective changes to GitHub and confirm that the expected files are all present on the GitHub website.  The view that you see when browsing GitHub is the same view that the grading TA sees when downloading your codebase.  

17. Wrap up the content portion of your homework by creating a `README.md` file in the `hw00` directory.  Include your name and e-mail address in that file.  Add, commit, and push that file.

18. Confirm that all files are present on GitHub.  Look at the commit history to find the full hash (see the Copy Full SHA hash button).  Note that for any commit that you can see all of the changes for the files associated with any commit. 

19. Copy the hash either from the GitHub website or run `git status` to see a list of all hashes.  Submit the hash from the commit via Canvas as the submission for your assignment.  

### Checklist - Task 4

You should have successfully done the following:

* Create an `hw/hw00` sub-directory in your private repository
* Populated / modified the files in that directory including `gitinfo.txt`, `hello.py`, `clever.txt`, and `README.md`
* Done at least three commits starting each commit message with `HW00`
* Confirmed that all files are present when viewed via GitHub
 
## Submission

Submit the final hash for your submission via the text field in Canvas for Homework 0.  Remember that the submission time is the later of the Canvas submission and the GitHub commit time.

## Rubric - Homework 00

* 1 pt - Completion of the Google Form with the correct information
* 3 pts - Homework submission is problem free without intervention required by the TA including files in the correct locations (`hw/hw00`) off of the private repository 
* 1 pts - At least three commit messages in the proper format
* 2 pts - All files are present as request (0.5 pts each) and modified as requested
   


   

 