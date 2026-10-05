// this file handle the built in commands

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../include/builtins.h"

// TODO LIST OF BUILT IN COMMANDS : 
//? [x] exit 
//? [x] cd 
//! [ ] history
//! [ ] export 

// handling the built in commands
// returns 0 if not builtin command
// returns 1 if it was builtin command and was handled
int handle_builtins(char *user_input[]){

    // command exit

    // if the user input is "exit" we exit the shell
    if(strcmp(user_input[0] ,"exit") == 0){
        exit(0);
        // command was built in and handled
        return 1;
    }

    // command cd

    // if the user input is "cd" we change the directory
    if (strcmp(user_input[0], "cd") == 0) {

        // defining the path user gave us as its destination
        char *path = user_input[1];

        // If 'cd' is called with no arguments,we go to HOME
        if (path == NULL) {
            path = getenv("HOME");
        }

        // if user has given us a directory to change to we change the directory
        // chdir() returns 0 on success, -1 on failure
        if (chdir(path) != 0) {
            // when we get error we show the error message to user and move on
            perror("cd failed");
        }

        // command was built in and handled
        return 1;
    }

    // command history

    // command export


    // command was not built in
    return 0;
}