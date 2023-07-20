# Unix-shell-implementation-Wish
The idea of this program is to work as a relatively simple Unix shell (CLI) prompt (like Bash for example). 
The shell creates a child process that executes the commands the user gives as input, and after executing a task, the shell will ask next input.  

You may invoke the program in the following way:
1. prompt> ./wish
- This will start the program and you may now give Unix commands as input!

2. prompt> ./wish batch.txt
- This is the so-called "batch mode", and the program will execute all commands that are listed in the input file ("batch.txt" in this example).
- Note that this will not start "wish prompt" it just reads and executes all of the commands in the input file!

NOTE: The shell itself does not implement any commands. All it does is finds the executable commands in one of the directories specified by the path and create a process to run them. 
The initial shell path contains one directory: ´/bin'.


The program contains three different built-in commands:
1. exit: When the user types "exit", the shell calls the exit system call with parameter 0.
2. cd: This command changes the directory, it takes one argument.
3. path: The path command takes 0 or more arguments, with each argument separated by whitespace from the others.
         A typical usage would be like this: wish> path /bin /usr/bin, which would add /bin and /usr/bin to the search path of the shell.
         If the user sets the path to empty, then the shell should not be able to run any programs (except built-in commands).
         The path command always overwrites the old path with the newly specified path.


Few additional notes:
- Redirection is implemented. If the user wants to send output in a file instead of the console, the user can type "command > output.txt", for example, ls -la > output.txt.
  There is an additional feature related to redirection: the standard error output of the file is rerouted to the file output.
  In addition, do note that redirection is not tested using built-in commands!
- Parallel commands are implemented. This can be used with an ampersand operator as follows: wish> cmd1 & cmd2 args1 args2 & cmd3 args1.
  This will cause the program to run all of these processes in parallel, before waiting for any of them to complete.


For more details on how the functionalities are implemented in the comments of the source code!
