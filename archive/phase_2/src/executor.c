// this file executes the command entered by the user in the shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

// Needed for open() and file flags (O_CREAT, O_WRONLY, etc.)
#include <fcntl.h> 

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

    // // breaking the user input into tokens using space
    // char *token = strtok(user_input, " ");
    // // what happen is :
    // // if the user input is : "sudo apt update && sudo apt upgrade"
    // // it will get transformend into : "sudo\0apt update && sudo apt upgrade" (the first space was replaced by null)

    // // once the whole user input is broken into tokens we go through each token until we reach the end 
    // while (token != NULL){

    //     // we store each token in the argument array
    //     // for example in first iteration : "sudo\0apt update && sudo apt upgrade"
    //     // we stores : sudo as argument[0]
    //     arguments[argument_count] = token;
    //     // inc the argument count (so we move to next empty space in array to store next token)
    //     argument_count++;
    //     // now we need to separate another token for next iteration
    //     token = strtok(NULL, " ");
    //     // now the user input is : "sudo\0apt\0update && sudo apt upgrade"
    //     // and it will read from first null to next null -> apt
    // }
    // // making the last element of argument array as NULL needed by execvp to know command has ended
    // arguments[argument_count] = NULL;

    // a flag : 0 means not in quote ; 1 means in quote
    int in_quotes = 0;
    
    // mark the very first argument at the start of user_input
    arguments[argument_count++] = user_input;

    for (int i = 0; user_input[i] != '\0'; i++) {
        
        // if we hit a quote -> flip our state
        if (user_input[i] == '"' || user_input[i] == '\'') {
            in_quotes = !in_quotes;
        }
        // if we hit a space AND we are NOT inside quotes, replace space with '\0'
        else if (user_input[i] == ' ' && !in_quotes) {
            user_input[i] = '\0'; // Cut the string here!
            
            // The next argument starts at the very next character
            arguments[argument_count++] = &user_input[i + 1];
        }
    }
    //! at the end we just want to convert user_input that looks like this :
    //* user_input : " git commit -m "commit message with spaces" "
    //! into :
    //* for execvp : " git " \0 " commit " \0 "-m " \0 ""commit message with spaces"" \0 NULL

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

    //? check for redirection ('>' or '>>')
    // creating a NULL pointer for a file as we dont know yet if user has used the > or >> 
    char *output_file = NULL;
    // flag variable : 0 = None, 1 = '>', 2 = '>>'
    int redirect_mode = 0; 

    // loop through the argument array until we hit the end (NULL)
    for (int i = 0; arguments[i] != NULL; i++) {

        // check if the current argument is exactly ">"
        if (strcmp(arguments[i], ">") == 0) { // if yes this condition works

            // we flag 1 to remember it is >
            redirect_mode = 1;
            // file name always comes just after >
            output_file = arguments[i + 1]; 
            // We replace ">" with NULL in the array!
            // Because execvp needs to run only "ls -l", NOT "ls -l > file.txt".
            // Setting arguments[i] to NULL makes execvp stop reading here and we can handle further
            arguments[i] = NULL;

            // we found the redirection symbol no need to loop anymore
            break;
        } 

        // check if the current argument is exactly ">>"
        else if (strcmp(arguments[i], ">>") == 0) { // if yes this condition works
            
            // we flag 2 to remember that >> was used
            redirect_mode = 2;
            // file name always comes just after >>
            output_file = arguments[i + 1]; 
             // We replace ">>" with NULL in the array!
            // Because execvp needs to run only "ls -l", NOT "ls -l >> file.txt".
            // Setting arguments[i] to NULL makes execvp stop reading here and we can handle further
            arguments[i] = NULL;
            
            // we found the redirection symbol no need to check anymore
            break;
        }

        // if no redirection symbol is found the flag stays 0
    }

    // If user wrote "ls >" without giving a filename (can happen and should not give error)
    if (redirect_mode != 0 && output_file == NULL) {
        fprintf(stderr, "Syntax error: missing file after redirection\n");
        return;
    }

    // creating a clone of our existing command : parent command - child command
    pid_t process_id = fork();

    // key concept is that we keep our parent command alive while out child command runs in the system
    // and gives its output,
    // we take that output and treat it as output of our parent command

    // process_id is 0 for child command : we run it
    if (process_id == 0){

        // apply redirection inside child process
        // if flag is 0 there was no redirection used so we dont do any of this
        // if the flag is not 0 (means its either 1 or 2) the redirection was used and we perform this code
        if (redirect_mode != 0) {

            // these are linux file flags...from fcntl.h library 
            // 0_WRONLY : Opens the file for WRite ONLY (no need to read it)
            // O_CREAT  : If the file does not exist CREATe it and Open
            int flags = O_WRONLY | O_CREAT; 

            // if the redirect mode is > it means write and >> means append
            if (redirect_mode == 1) { // > removes the existing content in file so write clean
                // O_TRUNC : means TRUNCATE the file (wipe and write) 
                flags |= O_TRUNC;  
            } else if (redirect_mode == 2) { // >> appends content at bottom of file
                // O_APPEND : means move to end of file and then write
                flags |= O_APPEND; 
            }

            // open the file in the disk
            // output_file : the name of the file (e.g. "output.txt")
            // flags       : the rules we just configured above
            // 0644        : standard Unix file permissions (owner can read and write; everyone else can read)
            // open() returns a File Descriptor number (e.g. 3) pointing to our file.
            int fd = open(output_file, flags, 0644);
            // If open() returned -1, something failed (e.g., folder is write-protected or disk is full rare but can happen)
            if (fd < 0) {
                perror("Redirection open error");
                exit(1);
            }

            // THE CORE SWITCHEROO: dup2(fd, STDOUT_FILENO)
            // STDOUT_FILENO is literally the number 1 (the standard output screen descriptor...means when dup2 is 1 the output is shown in the screen).
            // dup2 says: "Close Door 1 (the screen), and make door 1 point to our file (fd) instead"
            dup2(fd, STDOUT_FILENO);

            // close the file ....good practice
            close(fd);
        }

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
//? DONE line 48 to 66 in this file
// TODO: 1. Measure start and end time using clock_gettime(CLOCK_MONOTONIC, ...)
//? Done it in the sea_sh.c
// TODO: 2. Capture child process exit status using WEXITSTATUS(status) and return it
// not done yet
// TODO: 3. (Optional) Auto-add '--color=auto' when running 'ls' so output gets colors
// not done yet