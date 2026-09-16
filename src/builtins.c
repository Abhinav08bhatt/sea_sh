#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../include/builtins.h"

void handle_builtins(char *user_input){

    if(strcmp(user_input ,"exit") == 0){
        exit(0);
    }
}