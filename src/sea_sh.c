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