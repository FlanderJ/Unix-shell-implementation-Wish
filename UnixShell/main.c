#include "shellLib.h"

int main (int argc, char *argv[]) {
    int argAmount, parallel = 1;
    char command[MAX_LINE],*cLineArguments[MAX_LINE],*fileCommand=NULL;
    char error_message[40] = "usage: ./wish or ./wish command.txt\n";
    char error_message2[30] = "Failed to set path.\n";
    char error_message3[30] = "Failed to open a file.\n";
    PARALLEL *allParallelCommands = NULL;
    FILE *file;
    size_t n = 0;
    signal(SIGUSR1,sig_handler);

    // Setting initial shell path:
    if ((setenv("PATH","`/bin'",1))!=0) {
        raise(SIGUSR1);
        write(STDERR_FILENO, error_message2, strlen(error_message2));
        exit(1);
    }

    // If no input arguments is given, ask commands from user:
    if (argc == 1) {
        // Call print of prompt:
        while (printPrtomt(command))  {
            // Parse command line, if no command continue:
            if ((argAmount = parseCommandLine(command,cLineArguments,&parallel))==0){
                continue;
            };

            // If command parser detected parallelism, run them "simultaneously":
            if (parallel != 1) {
                allParallelCommands = parseParallel(cLineArguments,argAmount,parallel);
                runParallel(allParallelCommands,parallel);
                free(allParallelCommands); // Free allocated memory after use
                parallel = 1; // After performing all parallel commands set parallel varible to 0 again.
                continue;
            }

            // Check if the command is build-in:
            if (checkBuildIn(cLineArguments,argAmount)==1) {
                continue;
            }   
            
            // Search for the command (if not found continue):
            if (checkAccess(command)==-1) {
                continue;
            }

            // Run given command with it's arguments:
            runSubprocess(cLineArguments,argAmount);
        }
    }

    // If a file containing commands is given, run them:
    else if (argc == 2) {
        if ((file = fopen(argv[1],"r"))==NULL) {
            raise(SIGUSR1);
            write(STDERR_FILENO, error_message3, strlen(error_message3));
            exit(1);
        }

        while ((getline(&fileCommand,&n,file))!=EOF) {
            fileCommand[strlen(fileCommand)] = '\0'; // Replace last value with NULL to remove \n from the end.
            // Parse command line, if no command continue:
            if ((argAmount = parseCommandLine(fileCommand,cLineArguments, &parallel))==0){
                continue;
            }
            
            // If command parser detected parallelism, run them "simultaneously":
            if (parallel != 1) {
                allParallelCommands = parseParallel(cLineArguments,argAmount,parallel);
                runParallel(allParallelCommands,parallel);
                free(allParallelCommands);
                parallel = 1; // After performing all parallel commands set parallel varible to 0 again.
                continue;
            }
            if (parallel != 0) {

            }
            // Check if the command is build-in:
            if (checkBuildIn(cLineArguments,argAmount)==1) {
                continue;
            }   
            // Search for the command (if not found continue):
            if (checkAccess(fileCommand)==-1) {
                continue;
            }
            
            // Run given command with it's arguments:
            runSubprocess(cLineArguments,argAmount);

        }
        fclose(file); /* Remember to close file! */
        free(fileCommand);
    }
    
    // If too many input arguments:
    else {
        raise(SIGUSR1);
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    return 0;
}
