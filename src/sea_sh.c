#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/executor.h"
#include "../include/prompt.h"

int main(){

    // we are setting a text limit for the user input
    char user_input[1024];

    // a time variable to track the time taken for each command execution
    double time_taken = 0.0;
    struct timespec start, end;

    // the main loop that goes on until the user exits the shell
    while(1){

        // we show the prompt : function is in prompt.c
        show_prompt(time_taken);
        
        // we read the user input from fgets
        char *input_status = fgets(user_input, sizeof(user_input), stdin);
        
        // checking if the user gave inputs like ctrl+d or ctrl+c
        if (input_status == NULL){
            // in that case we force kill the shell
            break;
        }
        
        // removing the "newline character" from the fgets input
        user_input[strcspn(user_input, "\n")] = 0;
        
        // starting the clock to track time of execution
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // executing the real command
        execute_command(user_input);   

        // stopping the timer after the command execution is done
        clock_gettime(CLOCK_MONOTONIC, &end);

        // defining time taken : in next loop show_prompt function will read this value
        time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    }
    return 0;
}