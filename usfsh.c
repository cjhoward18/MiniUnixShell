/* File:
 *    usfsh.c
 *
 * Purpose:
 *    Mini Unix Shell
 *
 * Compile:
 *    gcc -o usfsh usfsh.c 
 * Usage:
 *    
 * Input:
 *    None
 * Output:
 *    Simple shell commands.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include "usfsh.h"

/*--------------------------------------------------------------------
 * Function:    printPrompt
 * Purpose:     Print '$' and current working directory.
 * In arg:      N/A
 */
void printPrompt() {
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        write(1, cwd,strlen(cwd));
    }
    write(1, "$ ", 2);
}

/*--------------------------------------------------------------------
 * Function:    redirectOut
 * Purpose:     Manipulates the file descriptor table to write output to a file. 
 * In arg:      arguments: array of arguments, command: file im redirecting to.
 */
void redirectOut(char* arguments[], char* command) {
    int fd,id;

    if ((fd = open(command, O_TRUNC | O_CREAT | O_WRONLY | O_SYNC , 0644)) < 0) {
        printf("cannot open %s\n", command);
        exit(1);
    }

    if ((id = fork()) == 0) {
        close(1);
        dup(fd);
        close(fd);
        execvp(arguments[0], arguments);
        write(2, "command not found please try again \n ", 37);
        exit(1);
    }
    close(fd);
    id = wait(NULL);
    return;
}

/*--------------------------------------------------------------------
 * Function:    redirectIn
 * Purpose:     Manipulates the file descriptor table to read Input from a file. 
 * In arg:      arguments: array of arguments, command: file im redirecting from.
 */
void redirectIn(char* arguments[], char* command) {
    int fd,id;

    if ((fd = open(command, O_CREAT | O_RDONLY | O_SYNC , 0644)) < 0) {
        printf("cannot open %s\n", command);
        exit(1);
    }

    if ((id = fork()) == 0) {
        close(0);
        dup(fd);
        close(fd);
        execvp(arguments[0], arguments);
        write(2, "command not found please try again \n ", 37);
        exit(1);
    }
    close(fd);
    id = wait(NULL);
    return;
}

/*--------------------------------------------------------------------
 * Function:    singleCommand
 * Purpose:     Execute a single command with nor arguments.
 * In arg:      substring: the command to execute.
 */
void singleCommand(char substring[]) {
    char *nullexec[2];
    nullexec[1] = NULL;
    nullexec[0] = substring;
    int id;

    if ((id = fork()) == 0) {
        execvp(substring, nullexec);
        write(1, "command not found please try again \n ", 36);
        exit(0);
    } else {
        id = wait(NULL);
    }
}

/*--------------------------------------------------------------------
 * Function:    piper
 * Purpose:     pipes the output of the first process to the input of the second process.
 * In arg:      arguments: array of arguments, command: file im redirecting to. command1: second commands arguments
 */
void piper(char* arguments[], char* command, char* command1) {
    pid_t id;
    int files[2];
    pipe(files);
    char* com[3];

    com[0] = command;
    com[1] = command1;
    com[2] = NULL;

    if ((id = fork()) == 0) {
        close(files[0]);
        close(1);     
        dup(files[1]);
        close(files[1]);  
        execvp(arguments[0], arguments);
        close(1);
        close(files[1]);
        write(2, "command not found please try again \n ", 37);
                exit(1);
    }

    if ((id = fork()) == 0) {
        close(0);
        dup(files[0]);
        close(files[1]);
        close(files[0]);
        execvp(com[0], com);
        close(0);
        close(files[0]);
        write(2, "command not found please try again \n ", 37);
        exit(1);
    } else {
        close(files[1]);
        close(files[0]);
        id = wait(NULL);
        id = wait(NULL);
    }
}

/*--------------------------------------------------------------------
 * Function:    execute_Line
 * Purpose:     reads from the command line (stdin) and executes commands like a mini Unix Shell.
 * In arg:      N/A
 * Ret val:     boolean that determines if program exits or promps user for next command.
 */
bool execute_Line() {

    char buf[1];
    int chars = 0;
    int args = 0;
    char line[256];
    int j = 0;
    int ascii;
    int count = read(0, buf, 1);
    int id;

    while (count > 0) {
        line[chars] = buf[0];
        ascii = buf[0];

        if (ascii == 32 && chars != 0) {
            args++;
        }

        if(ascii == 10) {
            char substring[chars];
            memcpy(substring, &line[0], chars);
            substring[chars] = '\0';

            if (args == 0) {
                if (strcmp("exit", substring) == 0) {
                    exit(0);
                    return true;
                } else {
                    singleCommand(substring);
                    return false;
                }
            }

            char* arguments[args+2];
            arguments[args+1] = NULL;
            char *command = strtok(substring," ");
            int redirect = 0;
            int pipes = 0;
            int redirectin = 0;

            while(command != NULL) { 

                if (strcmp(command, ">") == 0) {
                    redirect = 1;
                    break;
                }

                if (strcmp(command, "<") == 0) {
                    redirectin = 1;
                    break;
                }

                if (strcmp(command, "|") == 0) {
                    pipes = 1;
                    break;
                }

                arguments[j] = command;
                j++;
                command = strtok(NULL, " ");
            }

            if (strcmp(arguments[0], "cd") == 0) { // Changes directory
                int fail = chdir(arguments[1]);
                if (fail < 0) {
                    write(1, "directory not found please try again \n ", 39);
                }
                return false;
            }

            if (pipes == 1) { //redirects output to another programs input.
                command = strtok(NULL, " ");
                char *command1 = strtok(NULL, " ");
                if (command1 != NULL) {
                    arguments[args-2] = NULL;
                } else {
                    arguments[args-1] = NULL;
                }
                piper(arguments,command,command1);
                return false;
            }

            if (redirect == 1) { //redirects output to file.
                arguments[args-1] = NULL;
                command = strtok(NULL, " ");
                redirectOut(arguments,command);
                return false;
            }

            if (redirectin == 1) { //redirects output to file.
                arguments[args-1] = NULL;
                command = strtok(NULL, " ");
                redirectIn(arguments,command);
                return false;
            }

            if (( id = fork()) == 0) {
                execvp(arguments[0], arguments);
                write(1, "command not found please try again \n ", 37);
                exit(0);

            } else {
                id = wait(NULL);
                return false;
            }
        }

        if (ascii != 10) { //Counts characters.
            if (ascii == 32 && chars == 0) {
            } else {
                chars++;
            }
        }
        count = read(0, buf, 1);
    }
    return true;
}

int main(int argc, char **argv) 
{
    bool done = false;
    while (!done) {
        printPrompt();
        done = execute_Line();
    }
    return 0;
}