#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/executor.h"
#include "../include/prompt.h"

int main(){

    char user_input[1024];

    while(1){

        show_prompt();

        char *input_status = fgets(user_input, sizeof(user_input), stdin);
        if (input_status == NULL){
            break;
        }

        user_input[strcspn(user_input, "\n")] = 0;
        
        execute_command(user_input);   
    }


    return 0;
}


// TODO: 1. Create variables in main() to track 'last_status' and 'exec_time'
// TODO: 2. Pass 'last_status' and 'exec_time' into show_prompt()
// TODO: 3. Update 'last_status' from execute_command() return value