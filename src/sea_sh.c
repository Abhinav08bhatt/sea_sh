#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/executor.h"
#include "../include/prompt.h"

int main(){

    char user_input[1024];

    double time_taken = 0.0;
    struct timespec start, end;

    while(1){

        show_prompt(time_taken);
        
        char *input_status = fgets(user_input, sizeof(user_input), stdin);
        if (input_status == NULL){
            break;
        }
        
        user_input[strcspn(user_input, "\n")] = 0;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        execute_command(user_input);   

        clock_gettime(CLOCK_MONOTONIC, &end);

        time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    }


    return 0;
}


// TODO: 1. Create variables in main() to track 'last_status' and 'exec_time'
// TODO: 2. Pass 'last_status' and 'exec_time' into show_prompt()
// TODO: 3. Update 'last_status' from execute_command() return value