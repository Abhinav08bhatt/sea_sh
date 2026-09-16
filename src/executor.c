#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../include/executor.h"
#include "../include/builtins.h"

void execute_command(char *user_input){
    
    char *arguments[64];
    int argument_count = 0;

    char *token = strtok(user_input, " ");

    while (token != NULL){
        arguments[argument_count] = token;
        argument_count++;
        token = strtok(NULL, " ");
    }
    arguments[argument_count] = NULL;

    if (arguments[0] == NULL){
        return;
    }

    handle_builtins(user_input);  

    pid_t process_id = fork();

    if (process_id == 0){
        int execution_status = execvp(arguments[0], arguments);
        
        if(execution_status == -1){
            perror("Command Failed ");
        }
        exit(1);
    }
    else if (process_id > 0) {
        waitpid(process_id, NULL, 0);
    }
    else{
        perror("Fork Failed ");
    }

}


// TODO: 1. Measure start and end time using clock_gettime(CLOCK_MONOTONIC, ...)
// TODO: 2. Capture child process exit status using WEXITSTATUS(status) and return it
// TODO: 3. (Optional) Auto-add '--color=auto' when running 'ls' so output gets colors