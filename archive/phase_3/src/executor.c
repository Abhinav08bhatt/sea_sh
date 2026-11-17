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
// Needed to temporarily turn off raw mode when reading '<<' (heredoc) input from user
#include "../include/terminal.h"

// helper function to split ONE command string into arguments[] (respecting quotes and extra spaces)
// why a helper? because with pipes ("ls | grep .c"), there are spaces before/after '|'
// skipping extra spaces makes sure arguments[0] is never an empty string ""
static int tokenize_command(char *cmd, char *arguments[]) {

    // number of arguments in this command
    int argument_count = 0;

    // a flag : 0 means not in quote ; 1 means in quote
    int in_quotes = 0;

    char *p = cmd;

    while (*p != '\0' && argument_count < 63) {

        // skip any spaces before or between arguments (only when NOT inside quotes)
        while (*p == ' ' && !in_quotes) {
            p++;
        }

        // if we reached the end of the string after skipping spaces, stop
        if (*p == '\0') {
            break;
        }

        // mark the start of this argument
        arguments[argument_count++] = p;

        // walk forward until we hit a space (outside quotes) or the end of the string
        while (*p != '\0' && (*p != ' ' || in_quotes)) {

            // if we hit a quote -> flip our state
            if (*p == '"' || *p == '\'') {
                in_quotes = !in_quotes;
            }
            p++;
        }

        // if we stopped at a space (outside quotes), cut the string here with '\0'
        if (*p == ' ') {
            *p = '\0'; // Cut the string here!
            p++;
        }
    }

    // making the last element of argument array as NULL needed by execvp to know command has ended
    arguments[argument_count] = NULL;

    return argument_count;
}

// the fuction that does the job
void execute_command(char *user_input){
    
    // if user inputs nothing...a possible accidental enter or something we do nothing (code returns to sea_sh.c and shows next prompt)
    if (user_input == NULL || strlen(user_input) == 0) {
        return;
    }

    //? STEP 1 : Split user_input by '|' (pipe) outside quotes
    // example : "cat file.txt | grep hello | wc -l"
    // becomes :
    // commands[0] = "cat file.txt "
    // commands[1] = " grep hello "
    // commands[2] = " wc -l"
    char *commands[16];
    int cmd_count = 0;
    int in_quotes = 0;

    commands[cmd_count++] = user_input;

    for (int i = 0; user_input[i] != '\0'; i++) {
        // if we hit a quote -> flip our state (so '|' inside echo "a | b" is NOT treated as a pipe)
        if (user_input[i] == '"' || user_input[i] == '\'') {
            in_quotes = !in_quotes;
        }
        // if we hit '|' outside quotes, cut the string and start the next command
        else if (user_input[i] == '|' && !in_quotes) {
            if (cmd_count >= 16) {
                fprintf(stderr, "Error: too many pipes\n");
                return;
            }
            user_input[i] = '\0'; // Cut the command here!
            commands[cmd_count++] = &user_input[i + 1];
        }
    }

    // check for pipe syntax errors like "ls |" or "| grep a" or "ls | | wc"
    if (cmd_count > 1) {
        for (int k = 0; k < cmd_count; k++) {
            char *check = commands[k];
            while (*check == ' ') {
                check++;
            }
            if (*check == '\0') {
                fprintf(stderr, "Syntax error: invalid pipe usage\n");
                return;
            }
        }
    }

    // tokenizing the user input into arguments
    // this is a array with 64 elements
    // each element is a pointer to start of a string (char *)
    char *arguments[64];
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

    //! at the end we just want to convert user_input that looks like this :
    //* user_input : " git commit -m "commit message with spaces" "
    //! into :
    //* for execvp : " git " \0 " commit " \0 "-m " \0 ""commit message with spaces"" \0 NULL

    //? STEP 2 : Handle single-command built-ins in the parent process
    // (cd and exit MUST run in the parent shell when there is no pipe)
    if (cmd_count == 1) {
        argument_count = tokenize_command(commands[0], arguments);
        if (arguments[0] == NULL) {
            return;
        }
        // handling the built in commands : passing the whole argument array to function to deal with it
        if (handle_builtins(arguments) == 1) {
            // this means the command was built in and was handled now we go back to our main loop
            return;
        }
        // if this function returns...it means the command was not a built in command and we need to handle it as a normal command
    }

    //? STEP 3 : Execute all commands (works for both 1 command and multiple piped commands!)
    // prev_read_fd holds the read-end of the pipe from the PREVIOUS command
    // -1 means there is no previous command (so we are on the first command)
    int prev_read_fd = -1;

    // store all child PIDs so parent can wait for all of them AFTER the loop
    pid_t pids[16];
    int spawned_count = 0;

    // keep track of the last command name (for the cat newline fix at the bottom)
    char *last_cmd = NULL;

    for (int c = 0; c < cmd_count; c++) {

        // if cmd_count > 1, we haven't tokenized commands[c] yet (for cmd_count == 1 we already tokenized above)
        if (cmd_count > 1) {
            argument_count = tokenize_command(commands[c], arguments);
            if (arguments[0] == NULL) {
                if (prev_read_fd != -1) close(prev_read_fd);
                for (int w = 0; w < spawned_count; w++) waitpid(pids[w], NULL, 0);
                return;
            }
        }

        // if a command is not built it...it will be handled here:

        //? check for redirection ('>', '>>', '<', or '<<')
        // creating a NULL pointer for output file as we dont know yet if user has used > or >>
        char *output_file = NULL;
        // flag variable : 0 = None, 1 = '>', 2 = '>>'
        int out_redirect_mode = 0; 

        // creating NULL pointers for input redirection as we dont know yet if user has used < or <<
        char *input_file = NULL;
        // for '<<', the token right after '<<' is not a file name...it is a stop word (delimiter) like "EOF"
        char *heredoc_delimiter = NULL;
        // flag variable : 0 = None, 1 = '<', 2 = '<<'
        int in_redirect_mode = 0;

        // clean_idx keeps track of actual command arguments (excluding redirection symbols & filenames)
        // why do this instead of breaking early?
        // because user might combine BOTH input and output redirection on one line!
        // example : "sort < input.txt > output.txt" -> execvp should only receive ["sort", NULL]
        int clean_idx = 0;

        // loop through the argument array until we reach argument_count
        for (int i = 0; i < argument_count; i++) {

            // check if the current argument is exactly ">"
            if (strcmp(arguments[i], ">") == 0) { // if yes this condition works

                // we flag 1 to remember it is >
                out_redirect_mode = 1;
                // file name always comes just after >
                output_file = (i + 1 < argument_count) ? arguments[i + 1] : NULL;
                // skip the filename in the next iteration so it does not get passed to execvp
                i++;
            } 

            // check if the current argument is exactly ">>"
            else if (strcmp(arguments[i], ">>") == 0) { // if yes this condition works
                
                // we flag 2 to remember that >> was used
                out_redirect_mode = 2;
                // file name always comes just after >>
                output_file = (i + 1 < argument_count) ? arguments[i + 1] : NULL;
                // skip the filename in the next iteration so it does not get passed to execvp
                i++;
            }

            // check if the current argument is exactly "<" (input redirection)
            else if (strcmp(arguments[i], "<") == 0) {

                // we flag 1 to remember that < was used
                in_redirect_mode = 1;
                // input file name always comes just after <
                input_file = (i + 1 < argument_count) ? arguments[i + 1] : NULL;
                // skip the filename in the next iteration so it does not get passed to execvp
                i++;
            }

            // check if the current argument is exactly "<<" (here-document / heredoc)
            else if (strcmp(arguments[i], "<<") == 0) {

                // we flag 2 to remember that << was used
                in_redirect_mode = 2;
                // stop word (delimiter) always comes just after << (e.g. "EOF")
                heredoc_delimiter = (i + 1 < argument_count) ? arguments[i + 1] : NULL;
                // skip the delimiter in the next iteration so it does not get passed to execvp
                i++;
            }

            // if it is a normal command or flag (like "ls", "-l", "sort") we keep it for execvp
            else {
                arguments[clean_idx++] = arguments[i];
            }
        }

        // mark the end of the cleaned arguments array with NULL for execvp
        arguments[clean_idx] = NULL;

        // if there is no actual command left (e.g. user only typed "> file.txt") we do nothing
        if (arguments[0] == NULL) {
            if (prev_read_fd != -1) close(prev_read_fd);
            for (int w = 0; w < spawned_count; w++) waitpid(pids[w], NULL, 0);
            return;
        }

        last_cmd = arguments[0];

        // If user wrote "ls >" without giving a filename (can happen and should not give error)
        if (out_redirect_mode != 0 && (output_file == NULL || strlen(output_file) == 0)) {
            fprintf(stderr, "Syntax error: missing file after output redirection\n");
            if (prev_read_fd != -1) close(prev_read_fd);
            for (int w = 0; w < spawned_count; w++) waitpid(pids[w], NULL, 0);
            return;
        }
        // If user wrote "cat <" without giving an input file
        if (in_redirect_mode == 1 && (input_file == NULL || strlen(input_file) == 0)) {
            fprintf(stderr, "Syntax error: missing file after '<'\n");
            if (prev_read_fd != -1) close(prev_read_fd);
            for (int w = 0; w < spawned_count; w++) waitpid(pids[w], NULL, 0);
            return;
        }
        // If user wrote "cat <<" without giving a delimiter (like EOF)
        if (in_redirect_mode == 2 && (heredoc_delimiter == NULL || strlen(heredoc_delimiter) == 0)) {
            fprintf(stderr, "Syntax error: missing delimiter after '<<'\n");
            if (prev_read_fd != -1) close(prev_read_fd);
            for (int w = 0; w < spawned_count; w++) waitpid(pids[w], NULL, 0);
            return;
        }

        //? Handle '<<' (heredoc) BEFORE forking
        // a pipe is a temporary tunnel in memory with two ends:
        // heredoc_pipe[0] = read end (child command will read our typed text from here)
        // heredoc_pipe[1] = write end (parent shell will write the user's lines into here)
        int heredoc_pipe[2] = {-1, -1};
        if (in_redirect_mode == 2) {
            if (pipe(heredoc_pipe) == -1) {
                perror("Heredoc pipe failed");
                if (prev_read_fd != -1) close(prev_read_fd);
                for (int w = 0; w < spawned_count; w++) waitpid(pids[w], NULL, 0);
                return;
            }

            //! IMPORTANT : our shell runs in raw mode (ECHO and ICANON are OFF in terminal.c)
            // if we read with fgets() in raw mode, user won't see characters they type and backspace won't work
            // so we temporarily disable raw mode while collecting the heredoc lines
            disable_raw_mode();

            char line[1024];
            while (1) {
                // show a mini prompt so user knows shell is waiting for heredoc lines
                printf("> ");
                fflush(stdout);

                // read one line from user; if user presses Ctrl+D (EOF), fgets returns NULL
                if (fgets(line, sizeof(line), stdin) == NULL) {
                    // clear EOF flag on stdin so future input works normally
                    clearerr(stdin);
                    printf("\n");
                    break;
                }

                // temporarily strip trailing '\n' so we can compare cleanly against heredoc_delimiter (e.g. "EOF")
                size_t len = strlen(line);
                int has_newline = (len > 0 && line[len - 1] == '\n');
                if (has_newline) {
                    line[len - 1] = '\0';
                }

                // if the line matches the delimiter exactly, we stop reading heredoc input
                if (strcmp(line, heredoc_delimiter) == 0) {
                    break;
                }

                // put the '\n' back because the command (like cat or sort) expects normal lines with newlines
                if (has_newline) {
                    line[len - 1] = '\n';
                }

                // push this line into the write end of the pipe
                write(heredoc_pipe[1], line, strlen(line));
            }

            // switch terminal back to raw mode for our main sea_sh input loop
            enable_raw_mode();

            // close the write end of the pipe in parent
            // (VERY IMPORTANT : if write end stays open, the child command will wait forever thinking more data is coming!)
            close(heredoc_pipe[1]);
        }

        //? Create a pipe to connect THIS command to the NEXT command (only if this is NOT the last command)
        // pipefd[0] = read end  (next command will read from here)
        // pipefd[1] = write end (this command will write its output here)
        int pipefd[2] = {-1, -1};
        if (c < cmd_count - 1) {
            if (pipe(pipefd) == -1) {
                perror("Pipe Failed");
                if (prev_read_fd != -1) close(prev_read_fd);
                if (in_redirect_mode == 2) close(heredoc_pipe[0]);
                for (int w = 0; w < spawned_count; w++) waitpid(pids[w], NULL, 0);
                return;
            }
        }

        // creating a clone of our existing command : parent command - child command
        pid_t process_id = fork();

        // key concept is that we keep our parent command alive while out child command runs in the system
        // and gives its output,
        // we take that output and treat it as output of our parent command

        // process_id is 0 for child command : we run it
        if (process_id == 0){

            // 1. If there is a previous command's pipe, wire STDIN (0) to read from it
            if (prev_read_fd != -1) {
                dup2(prev_read_fd, STDIN_FILENO);
                close(prev_read_fd);
            }

            // 2. If there is a NEXT command, wire STDOUT (1) to write into the new pipe
            if (c < cmd_count - 1) {
                // child does not read from its own outgoing pipe, so close read end
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            // 3. apply input redirection inside child process ('<' or '<<')
            // (runs AFTER pipe wiring so explicit '<' or '<<' overrides pipe input if both exist)
            if (in_redirect_mode == 1) {
                // O_RDONLY : Opens the file for ReaD ONLY (we are feeding this file as input to the command)
                int fd_in = open(input_file, O_RDONLY);
                // If open() returned -1, file probably doesn't exist or has no read permission
                if (fd_in < 0) {
                    perror("Input redirection open error");
                    exit(1);
                }

                // THE CORE SWITCHEROO FOR INPUT: dup2(fd_in, STDIN_FILENO)
                // STDIN_FILENO is literally the number 0 (standard input / keyboard).
                // dup2 says: "Close Door 0 (the keyboard), and make door 0 read from our file (fd_in) instead"
                dup2(fd_in, STDIN_FILENO);

                // close the extra file descriptor ....good practice
                close(fd_in);
            }
            else if (in_redirect_mode == 2) {
                // Make Door 0 (STDIN_FILENO) read from the read-end of our heredoc pipe instead of the keyboard
                dup2(heredoc_pipe[0], STDIN_FILENO);

                // close the original read-end fd now that STDIN_FILENO points to it
                close(heredoc_pipe[0]);
            }

            // 4. apply output redirection inside child process ('>' or '>>')
            // if flag is 0 there was no output redirection used so we dont do any of this
            // if the flag is not 0 (means its either 1 or 2) the redirection was used and we perform this code
            if (out_redirect_mode != 0) {

                // these are linux file flags...from fcntl.h library 
                // O_WRONLY : Opens the file for WRite ONLY (no need to read it)
                // O_CREAT  : If the file does not exist CREATe it and Open
                int flags = O_WRONLY | O_CREAT; 

                // if the redirect mode is > it means write and >> means append
                if (out_redirect_mode == 1) { // > removes the existing content in file so write clean
                    // O_TRUNC : means TRUNCATE the file (wipe and write) 
                    flags |= O_TRUNC;  
                } else if (out_redirect_mode == 2) { // >> appends content at bottom of file
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

            // if a built-in (like cd) is used inside a pipe, handle it in this child and exit cleanly
            if (cmd_count > 1 && handle_builtins(arguments) == 1) {
                exit(0);
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
        // process_id of parent command is always above 0 : we close unused pipe ends and save PID to wait later
        else if (process_id > 0) {
            pids[spawned_count++] = process_id;

            // parent is done with the previous pipe's read end (the child now owns it)
            if (prev_read_fd != -1) {
                close(prev_read_fd);
            }

            // parent does not need the read end of the heredoc pipe, so close it to prevent fd leaks
            if (in_redirect_mode == 2) {
                close(heredoc_pipe[0]);
            }

            // if we created a pipe for the next command:
            // 1. close the write end in parent (CRITICAL: otherwise next command hangs waiting for EOF!)
            // 2. save the read end so the next command in the loop can read from it
            if (c < cmd_count - 1) {
                close(pipefd[1]);
                prev_read_fd = pipefd[0];
            } else {
                prev_read_fd = -1;
            }
        }
        // rare but happens when memory is full...then we need to say that we could not fork
        else{
            if (prev_read_fd != -1) close(prev_read_fd);
            if (in_redirect_mode == 2) close(heredoc_pipe[0]);
            if (c < cmd_count - 1) {
                close(pipefd[0]);
                close(pipefd[1]);
            }
            perror("Fork Failed ");
            break;
        }
    }

    // wait for ALL child commands in the pipeline to finish
    for (int w = 0; w < spawned_count; w++) {
        waitpid(pids[w], NULL, 0);
    }

    //! CHEATING : to fix the newline bug (works 60% of the cases but does not fix the cause)
    if(last_cmd != NULL && strcmp(last_cmd, "cat") == 0){
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