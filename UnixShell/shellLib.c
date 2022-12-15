#include "shellLib.h"

// This function prints shell prompt and takes the user input and returns it as command:
char *printPrtomt(char *command) {
    size_t n = MAX_LINE;
    printf("wish> ");

    // Get user input:
    if (getline(&command,&n,stdin) != EOF) { 
        command[strlen(command)-1] = '\0'; // Replace last value with \0 to remove \n from the end.
    }

    return command;
}

// This function handles error signals:
void sig_handler(int signum) {
    char error_message[30] = "An error has occured.\n";
    if (signum == 10) {
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
}

// This function parses user commands, removes all white-spaces ands also "detects" if there is parallelism symbol given:
int parseCommandLine(char *command, char **cmdPtr, int *parallel) {
    int i=0; 
    char *arg;
    // Check all white-spaces from beginning of the command:
    while (isspace(command[0]) != 0) {
        memmove(&command[0],&command[1], strlen(command)); // remove white-space
    }   
    // Read user input (interpret command):
    if ((arg = strtok(command," "))==NULL) {return 0;}; // command
    while (isspace(arg[strlen(arg)-2]) != 0) {
        memmove(&arg[strlen(arg)-2],&arg[(strlen(arg)-2)+1], strlen(arg)-strlen(arg)-2); // remove white-space
    }
    removeWhiteSpace(arg); // Remove all whitespaces from the end of the command
    cmdPtr[i++] = arg; // Write the command into memory
    // Arguments:
    while ((arg = strtok(NULL," ")) != NULL) {
        if (arg[strlen(arg)-1] == '\0') { // If the last char is \0 then we are at the end of the commandline.
            break;
        }
        // Check all white-spaces from beginning of argument:
        while (isspace(arg[0]) != 0) {
            memmove(&arg[0],&arg[1], strlen(arg)); // remove white-space
        }   
        removeWhiteSpace(arg); // Remove all whitespaces from the end of the command or argument

        // Detect parallelism:
        if (strcmp(arg,"&") == 0) {
            *parallel=*parallel+1; // Count how many parallel commands
        }
        if(strcmp(arg,"")==0) {
            continue;
        }
        cmdPtr[i] = arg; // Save arguments and commands
        i++;
    }
    // Last entry of arguments set to NULL:
    cmdPtr[i] = NULL;
    return (i);
}

// This function removes all white-spaces from the end of the input string:
void removeWhiteSpace(char *string) {
    int i = strlen(string) - 1;
    while (i > 0) {
        if (string[i] == ' ' || string[i] == '\n' || string[i] == '\t' || string[i] == '$') {
            i--;
        }
        else break;
    }
    string[i+1] ='\0';
    return;
}

// This function parses command line into separate command lines into vectors regarding to the command amount and adds their arguments into the same vector
PARALLEL *parseParallel(char **cmdPtr, int argAmount, int parallel) {
    int newProgmarindex=0,k=0,j=0;
    char error_message[30] = "Malloc failed.\n";
    PARALLEL *parallelCommands=NULL;
    if ((parallelCommands = malloc(sizeof(PARALLEL)))==NULL) {
        raise(SIGUSR1);
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
    }
   
    /* String matrix for different commands and their arguments (NOTE: This is not the most efficient way of saving data, 
    but it fills requirements of the assignment! Just wanted to try something different and learn how string matrix on struct would work on c). */
    for (k=0;k<argAmount;k++) {
        if (strcmp(cmdPtr[k],"&")==0) { // Detect when next command starts
            (parallelCommands->commandLines[newProgmarindex])[j] = NULL; // last argument must be NULL for execvp
            parallelCommands->argAmounts[newProgmarindex] = j; // Also save argument amount!
            newProgmarindex++; // Because we encountered ampersand, we move to the next vector
            j=0;
            continue;;
        }
        (parallelCommands->commandLines[newProgmarindex])[j] = cmdPtr[k];
        j++;
    }
    (parallelCommands->commandLines[newProgmarindex])[j] = NULL; // last argument must be NULL for execvp
    parallelCommands->argAmounts[newProgmarindex] = j; // Also save argument amount for last command
    return parallelCommands;
    }

// This function creates threads and initiates them:
void runParallel(PARALLEL *parallelCommands, int parallelAmount) {
    int i;
    char error_message1[30] = "Thread creation failed.\n";
    char error_message2[30] = "Thread join failed.\n";
    pthread_t th[parallelAmount];
    parallelCommands->parallelProgCount = 0; // Set counter to 0;
    // Create all threads:
    for (i=0;i<parallelAmount;i++) {
        if (pthread_create(&th[i],NULL,&initiateCommands,(void*)parallelCommands)!=0){
            raise(SIGUSR1);
            write(STDERR_FILENO, error_message1, strlen(error_message1));
            return;
        }
    }
    // Initiate all threads (in separate loop to make them run parallel):
    for (i=0;i<parallelAmount;i++) {
        if (pthread_join(th[i],NULL)!=0){
            raise(SIGUSR1);
            write(STDERR_FILENO, error_message2, strlen(error_message2));
            return;
        }
    }
    return;
}

/* This is thread function, which will handle each command and run them the same way as 
the commands would have been ran if they were typed separedly, but instead they all run "at the same time".*/
void *initiateCommands(void *allCommands) {
    PARALLEL *allParallelComs = (PARALLEL*)allCommands;
    // Select the program run in cur thread:
    int i = (*allParallelComs).parallelProgCount;
    (*allParallelComs).parallelProgCount++; // Increase for the next thread, to avoid same program run repeatedly
    // Check if the command is build-in:
    if (checkBuildIn((*allParallelComs).commandLines[i],(*allParallelComs).argAmounts[i])==1) {
        return 0; // return if it is
    }   
    if (checkAccess(((*allParallelComs).commandLines[i])[0]) == -1) { // If the command is not found from the path return
        return 0;
    }
    runSubprocess((*allParallelComs).commandLines[i],(*allParallelComs).argAmounts[i]); // Run command
    return 0;
}

/* This function runs all commands given to it as input, by creating a child process, which will be replaced with new process by exec defined by user on input*/
void runSubprocess(char **cmdPtr, int argAm) {
    int status,pid,i, output_fd;
    char error_message1[30] = "Fork failed.\n";
    char error_message2[50] = "Redirection usage: 'command' > 'outputfilename'.\n";
    char error_message3[30] = "Execvp failed.\n";
    char error_message4[30] = "Wait failed.\n";
    // Create a child by using fork():
    if ((pid = fork())==-1) {
        raise(SIGUSR1);
        write(STDERR_FILENO, error_message1, strlen(error_message1));
        exit(1);
    }
    // Child:
    else if (pid==0) {
        // Check redirection:
        for (i=1;i<argAm;i++) {
            // Handle output and error redirection to same file:
            if (strcmp(cmdPtr[i],">")==0) {
                output_fd = open(cmdPtr[++i], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR); /*A file where to redirect*/
                dup2(output_fd,STDOUT_FILENO);  /*Redirect to stdout*/
                dup2(output_fd,STDERR_FILENO);  /*Redirect to stderr*/
                close(output_fd);

                // Check for errors(NOTICE: These error message will be printed into given output file!!):
                if (i+1 != argAm) {   
                    raise(SIGUSR1);
                    write(STDERR_FILENO, error_message2, strlen(error_message2));
                    myExit();
                }

                cmdPtr[--i] = NULL; // Set the character ">" to NULL
                if (execvp(cmdPtr[0], cmdPtr) == -1) {
                    raise(SIGUSR1);
                    write(STDERR_FILENO, error_message3, strlen(error_message3));
                    exit(1);
                }
                break;
            }
        }
        
        // No redirections:
        if (execvp(cmdPtr[0], cmdPtr) == -1) {
            raise(SIGUSR1);
            write(STDERR_FILENO, error_message3, strlen(error_message3));
            exit(1);
            }
        }
    
    // Parent:
    else {
        if (wait(&status) == -1) {
            raise(SIGUSR1);
            write(STDERR_FILENO, error_message4, strlen(error_message4));
            exit(1);
        }
    }  
    
    return;
}

// Checks path for called program, if not found return -1, if found return 0.
int checkAccess(char *command) {
    char error_message[50] = "No such command in path.\n";
    char *pathPtr,temp[MAX_LINE],*paths[MAX_LINE]={"path","/bin/","/usr/bin/"};
    // If the path is not set:
    if ((strcmp(getenv("PATH"),"`/bin'")) == 0) {
        myPath(paths,3);
    }
    // Traverse all paths and try to find corresponding program:
    if ((pathPtr = strtok(getenv("PATH"),":"))!= NULL) {
        strcpy(temp,pathPtr);
        strcat(temp,command);
        if ((access(temp,X_OK))==0) {
            return 0;
        }
    }
    
    while ((pathPtr = strtok(NULL,":"))!= NULL) {
        strcpy(temp,pathPtr);
        strcat(temp,command);
        if ((access(temp,X_OK))==0) {
            return 0;
        }
    }
    // Not found.
    raise(SIGUSR1);
    write(STDERR_FILENO, error_message, strlen(error_message));
    return -1;   
}

// Check if the command is build-in, if not return 0, if it is return 1.
int checkBuildIn(char **cmdPtr,int argAmount) {
    char error_message[40] = "Command exit takes no input arguements.\n";

    // If the command is exit terminate the process:
    if ((strcmp(cmdPtr[0],"exit"))==0){
        // Check if input arguments was given:
        if (argAmount != 1) {
            raise(SIGUSR1);
            write(STDERR_FILENO, error_message, strlen(error_message));
            return 1;
        }
        myExit();
    }

    // If command is build-in one, then run it:
    if ((strcmp(cmdPtr[0],"cd"))==0) {
        myCd(cmdPtr,argAmount);
        return 1;
    }
    else if ((strcmp(cmdPtr[0],"path"))==0) {
        myPath(cmdPtr,argAmount);
        return 1;
    }

    return 0;
}


// Build-in commands:
void myExit(void) {
    exit(0); 
}
void myCd(char *cmdPtr[],int argAmount) {
    char error_message1[50] = "Command 'cd' takes only one input arguement.\n";
    char error_message2[30] = "No such directory.\n";
    if (argAmount!=2) { /*Check for correct amount of input arguments*/
        raise(SIGUSR1);
        write(STDERR_FILENO, error_message1, strlen(error_message1));
        return;
    }
    if ((chdir(cmdPtr[1])) == -1) { // Change directory.
        raise(SIGUSR1);
        write(STDERR_FILENO, error_message2, strlen(error_message2));
    }
    return;
}
void myPath(char *cmdPtr[],int argAmount) {
    char paths[MAX_LINE]="";
    char error_message[30] = "Failed to set path\n";
    int i;

    // No arguments, set path empty:
    if (cmdPtr[1]==NULL) {
        if ((setenv("PATH"," ",1))!=0) {
            raise(SIGUSR1);
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }  
    }

    // Form path string:
    for (i=1;i<argAmount;i++) {
        strcat(paths,cmdPtr[i]);
        // Break if last iteration:
        if (i == (argAmount-1)) {
            break;
        }
        strcat(paths,":");
    }
    // Set new path:
    if ((setenv("PATH",paths,1))!=0) {
        raise(SIGUSR1);
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }  

    return;
}
