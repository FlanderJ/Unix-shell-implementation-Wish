#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE 
#define MAX_LINE 255
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <sys/utsname.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <ctype.h>

/* (NOTE: This is not the most efficient way of saving data, but it fills requirements of the assignment!
Just wanted to try something different and learn how string matrix on struct would work on c). */
typedef struct parallelInput{
    int argAmounts[MAX_LINE];
    int parallelProgCount;
    char *commandLines[MAX_LINE][MAX_LINE];
} PARALLEL;


char *printPrtomt(char *command);
void sig_handler(int signum);
int parseCommandLine(char *command, char **cmdPtr,int *parallel); 
void removeWhiteSpace(char *string);
PARALLEL *parseParallel(char **cmdPtr, int argAmount, int parallel);
void runParallel(PARALLEL *parallelCommands, int parallelAmount); 
void *initiateCommands(void *allCommands); 
void runSubprocess(char **cmdPtr, int argAm);
int checkAccess(char *command);
int checkBuildIn(char **cmdPtr,int argAmount);
void myExit(void);
void myCd(char *cmdPtr[],int argAmount);
void myPath(char *cmdPtr[], int argAmount);
