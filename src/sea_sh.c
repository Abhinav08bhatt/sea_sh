// when we run the shell we will be in this file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// IMPORTANT : it fixs the killing of the terminal on ctrl+c or other OS force
#include <signal.h>

#include "../include/executor.h"
#include "../include/prompt.h"
#include "../include/terminal.h"
#include "../include/history.h"


int main(){

    // from the signal.h library : asking our code to ignore interrupt signals
    //! SIGINT : SIGnal INTerrupt (signal sent by OS when we press ctrl+c to interrupt a process)
    //! SIG_IGN : SIGnal IGNore (it tells the system to ignore this specific signal)
    signal(SIGINT, SIG_IGN);

    // we runs this function from terminal.c file
    // we enables the raw mode of terminal
    enable_raw_mode();

    // reading the commands from the storage file :
    // extracting the commands from file to the runtime linked-list
    init_history();

    // creating space for user_input in the memory
    char user_input[1024];
    
    // creating a variable to track time
    double time_taken = 0.0;
    // needed for time calculation
    struct timespec start, end;

    // creating a history pointer (the current command)
    Node *history_ptr = NULL;

    // the main loop that runs as long as user is in shell
    while(1){

        // showing the prompt to the user (prompt.c file)
        show_prompt(time_taken);
        
        // reading the user input from terminal in raw mode

        // keeps track of the number of characters in user_input
        int i = 0;
        // keeps track of the cursor position in user_input (we need it to move left/right)
        int cursor_pos = 0;
        // reading the user input character by character
        char c;
        // clearing the user_input buffer (might be sometime present a \n or %)
        user_input[0] = '\0';
        
        // now we read every single character the user presses
        // we only stop reading when the user presses "enter" (caz now we have the whole command to work with)
        while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n') {

            // when we press arrow key it sends a escape sequence
            // ^[[A : up arrow
            // ^[[B : down arrow
            // ^[[C : right arrow
            // ^[[D : left arrow
            // in this we look at the first bit : ^[ : ESC : 27 in ascii (^[ is a single char that we read in one bit])
            if (c == 27) {

                // reading the second bit of the escape sequence : [ : 91 in ascii
                read(STDIN_FILENO, &c, 1);
                
                // reading the third bit of the escape sequence : c : A,B,C,D
                read(STDIN_FILENO, &c, 1);
                
                // we only focus on A and B (up and down arrow) for now
                if (c == 'A' || c == 'B') {

                    // showing the prompt again and clearing the line (in case we are showing a previous command)
                    // /r : moves the cursor to the start of line
                    // \33[K : clears the line from cursor to end of line
                    printf("\r\33[K 🐚 ❯ ");

                    // now we are handling the history
                    // the functions are in history.c file for working in doubly linked list

                    // up arrow : ^[[A
                    if (c == 'A') {
                        // if the arrow key is pressed for the first time
                        // there is no past command to show
                        // so we set history_ptr to last command
                        if (!history_ptr) {
                            history_ptr = get_last();
                        }
                        // if the pointer is not empty : we move to previous command in history
                        // we store the node of the previous command in history_ptr
                        else {
                            history_ptr = get_prev(history_ptr);
                        }
                    }
                    // down arrow : ^[[B
                    // if the down arrow key is pressed :
                    // we show the next command in history : 
                    // NULL is does not exist
                    // if the command exist we stores its node in history_ptr
                    else {
                        history_ptr = get_next(history_ptr);
                    }
                    
                    // now we have a node in history_ptr

                    // if the history_ptr is not NULL and valid
                    if (history_ptr) {
                        // we print the command stored in the node of history_ptr in terminal
                        printf("%s", history_ptr->command);
                        // we store the command in user_input so that we can execute it later
                        strcpy(user_input, history_ptr->command);
                        // storing the length of command so we can press left or right arrow and backspace
                        i = strlen(user_input);
                        // moves cursor to the end of command (so we can edit it)
                        cursor_pos = i;
                    } 
                    // if the history_ptr is NULL : there is no command to show
                    // and if there is a existing text in user_input we intentionally clear it and bring cursor to 0
                    else {
                        user_input[0] = '\0';
                        i = 0;
                        cursor_pos = 0;
                    }
                } 

                // handling left and right arrow key

                // if user press right arrow key : ^[[C
                // we move the cursor to right if it is not at the end of command
                else if (c == 'C') { 
                    if (cursor_pos < i) {
                        printf("\033[C");
                        cursor_pos++;
                    }
                } 
                // if user press left arrow key : ^[[D
                // we move the cursor to left if it is not at the start of command
                else if (c == 'D') {
                    if (cursor_pos > 0) {
                        printf("\033[D");
                        cursor_pos--;
                    }
                }

                // clearing the buffer (just to keep it clean)
                fflush(stdout);
            }

            // handling backspace key : 127 in ascii
            else if (c == 127) {
                // if cursor position is more then 0 : this means there is content in user_input which we can backspace through
                if (cursor_pos > 0) {
                    // we take the cursor position and we shift everything cursor to its left
                    // replacing the current cursor position with next text (replicating backspace) (visually only) (+1 for the last null character)
                    memmove(&user_input[cursor_pos - 1], &user_input[cursor_pos], i - cursor_pos + 1);
                    // we dec the length of input 
                    i--;
                    // we dec the internal cursor tracker one position back
                    cursor_pos--;
                    // \b : more the literal terminal cursor one step back
                    // \33[K : erases the literal old character that was on screen
                    printf("\b\33[K");

                    // showing the characters that were right side from the deleted character to now position
                    printf("%s", &user_input[cursor_pos]);
                    
                    // now after showing the characters again our literal terminal cursor moves to end of the line
                    // to bring it back where it was we back it uing \b "backspace escape sequence"
                    for (int j = 0; j < (i - cursor_pos); j++){
                        printf("\b");
                    }
                    
                    // clearing the buffer (just to keep it clean)
                    fflush(stdout);
                }
            } 

            // if there was no arrow key pressed no backspace pressed : it means it was a normal character that is a command user is trying to input
            else {
                // if a normal elem is pressed we shift everything on its right to 1 step making a hole for our new elem
                // example : [a],[b],[c],[NULL] and my cursor is after [c] it does : [a],[b],[c],[ ],[NULL] 
                // example : [a],[b],[d],[NULL] and my cursor is after [b] it does : [a],[b],[ ],[d],[NULL] 
                // (this happens in the memory not visually on terminal)
                memmove(&user_input[cursor_pos + 1], &user_input[cursor_pos], i - cursor_pos + 1);
                // putting the just typed user character into the hole
                user_input[cursor_pos++] = c;
                // inc the total number of characters in command
                i++;

                // visual handling :
                // if the user enters a character in middle of a command :
                // example : [a],[b],[c] and user enters it after [b] it will look like this : [a],[b],[ ] (the [c] was replaced by hole visually)
                // to handle it we d some steps :
                // we prints the new character in the cursor postion : [c],[a],[t] becomes [c],[o],[t] (a is replaced by o visually)
                putchar(c);
                // we saves the current cursor position (the position at [o])
                printf("\33[s");
                // we prints stuff after cursor postion : [c],[o],[a],[t] ([a],[t] was recalled from memory to be printed after the cursor position [o])
                printf("%s", &user_input[cursor_pos]);
                // brings cursor back to saved position (after printing [a],[t] our cursor will move to end which we dont want)
                printf("\33[u");

                // clearing the buffer (just to keep it clean)
                fflush(stdout);
            }
        }

        // the above while loop ends when we press the enter button : \n : newline key

        // we print the \n key to make rest of the program work in next line
        printf("\n");

        // if the user command length was more then 0 : means there was some text 
        // we create a new memory node for the command 
        if (i > 0){
            add_history(user_input);
        }
        // we defin the pointer again as NULL : current pointer for the next iteration of while loop
        history_ptr = NULL;
        
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