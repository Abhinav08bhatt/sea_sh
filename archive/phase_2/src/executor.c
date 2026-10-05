// this file executes the command entered by the user in the shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../include/executor.h"
#include "../include/builtins.h"

// the fuction that does the job
void execute_command(char *user_input){
    
    // tokenizing the user input into arguments
    // this is a array with 64 elements
    // each element is a pointer to start of a string (char *)
    char *arguments[64];
    // number of arguments in the user input
    int argument_count = 0;

    // breaking the user input into tokens using space
    char *token = strtok(user_input, " ");
    // what happen is :
    // if the user input is : "sudo apt update && sudo apt upgrade"
    // it will get transformend into : "sudo\0apt update && sudo apt upgrade" (the first space was replaced by null)

    // once the whole user input is broken into tokens we go through each token until we reach the end 
    while (token != NULL){

        // we store each token in the argument array
        // for example in first iteration : "sudo\0apt update && sudo apt upgrade"
        // we stores : sudo as argument[0]
        arguments[argument_count] = token;
        // inc the argument count (so we move to next empty space in array to store next token)
        argument_count++;
        // now we need to separate another token for next iteration
        token = strtok(NULL, " ");
        // now the user input is : "sudo\0apt\0update && sudo apt upgrade"
        // and it will read from first null to next null -> apt
    }
    // making the last element of argument array as NULL needed by execvp to know command has ended
    arguments[argument_count] = NULL;

    // if user inputs nothing...a possible accidental enter or something we do nothing (code returns to sea_sh.c and shows next prompt)
    if (arguments[0] == NULL){
        return;
    }

    // handling the built in commands : passing the whole argument array to function to deal with it
    if (handle_builtins(arguments) == 1) {
        // this means the command was built in and was handled now we go back to our main loop
        return;
    }
    // if this function returns...it means the command was not a built in command and we need to handle it as a normal command

    // if a command is not built it...it will be handled here:

    // creating a clone of our existing command : parent command - child command
    pid_t process_id = fork();

    // key concept is that we keep our parent command alive while out child command runs in the system
    // and gives its output,
    // we take that output and treat it as output of our parent command

    // process_id is 0 for child command : we run it
    if (process_id == 0){
        // we simply pass all the arguments to execvp and it handles them and executes them
        int execution_status = execvp(arguments[0], arguments);
        // if the execution is success...it shows the command output and does nothing
        // if the execution fails...it returns -1 and we show the error message
        if(execution_status == -1){
            perror("Command Failed ");
        }
        // no matter if the execution pass or fail we need to exit child command and handover the result to parent
        exit(1);
    }
    // process_id of parent command is always above 0 : we make it wait until the child is done with job
    else if (process_id > 0) {
        waitpid(process_id, NULL, 0);
    }
    // rare but happens when memory is full...then we need to say that we could not fork
    else{
        perror("Fork Failed ");
    }

    //! CHEATING : to fix the newline bug (works 60% of the cases but does not fix the cause)
    if(strcmp(arguments[0], "cat") == 0){
        printf("\n");
    }
}


// TODO: Fix tokenizer to respect quotes (don't split spaces if inside "..." or '...')
// not done yet
// TODO: 1. Measure start and end time using clock_gettime(CLOCK_MONOTONIC, ...)
//? Done it in the sea_sh.c
// TODO: 2. Capture child process exit status using WEXITSTATUS(status) and return it
// not done yet
// TODO: 3. (Optional) Auto-add '--color=auto' when running 'ls' so output gets colors
// not done yet