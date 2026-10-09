// imports :
#include <stdio.h> // for standard functions 
#include <stdlib.h> // for malloc
#include <string.h> // for string functions
#include "../include/history.h" // header file 

// using MACRO : can change the file name without chaning the code (might be needed in future)
#define HISTORY_FILE ".sea_sh_history" 

// initializing the first node as empty with no past and no future 
static Node *head = NULL;
static Node *tail = NULL;

// Internal helper: adds to the linked list WITHOUT writing to the file during the runtime (no use outside this file)
// (Used when loading history from the file at startup)
static void raw_add_history(char *cmd) { // static is used to define a function that can only be used inside a file and not outside it

    // no saving the blank interaction in file if the user pressed enter without writing anything 
    // (saves space and limits accidental touches)
    if (strlen(cmd) == 0) {
        return;
    }

    // defining a new empty node (using malloc)
    Node *new_node = malloc(sizeof(Node));
    
    
    //! WHY USE STRDUP ???
    // new_node->command = cmd;
    //? doing this : we will literally store the ADDRESS OF THE INPUT : 
    // meaning everytime we change our input, the content in linked list will change too (at the end every command will be a copy of last command) 
    
    // what STRDUP does : it creates a new memory block for the input in hand and puts the contnet inside of it automatically
    new_node->command = strdup(cmd);
    
    // defining the future : NULL (no future for the latest node)
    new_node->next = NULL;
    // defining past : current node becomes the past 
    // and linking the current node to the past 
    new_node->prev = tail;

    // if the past exist : 
    // connecting the past node form the current node
    if (tail) {
        tail->next = new_node;
    } 
    // if past does not exit : its the first node (so the new_node is also the first node)
    else {
        head = new_node;
    }
    // updating the global tail so it points towards current node
    tail = new_node;
}

// Load existing history from .sea_sh_history into the linked list
void init_history() {

    // opening the file in read format
    FILE *file = fopen(HISTORY_FILE, "r");

    // File doesn't exist yet... which is fine for the first run : 
    // we dont want to give error
    if (!file) {
        return; 
    }

    // creating a buffer to store 1024 characters :
    // the buffer stores the most recent line extracted from the file....
    // the extracted content is passed to the linked list working in the runtime... 
    // and every time the while loop below iterates...it overwrite the buffer on every line 
    char buffer[1024];

    // read line by line from the file
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // remove the trailing newline character if it exists
        buffer[strcspn(buffer, "\n")] = 0;
        
        // if the line is not empty we give the value inside the buffer to the linked list
        if (strlen(buffer) > 0) {
            raw_add_history(buffer);
        }
    }
    // closing the file...idk but good practice
    fclose(file);
}

// adds a command to the history linked list AND appends it to the file
void add_history(char *cmd) {
    
    // if the command is empty (nothing but enter or stuff) we dont consider it
    if (strlen(cmd) == 0) {
        return;
    }

    // adding the command in the runtime linked list (faster to access on the go)
    raw_add_history(cmd);

    // Append the command to the storage file
    // if the file dose not exist it created one on its own
    FILE *file = fopen(HISTORY_FILE, "a");
    
    // file opens and we write in it
    if (file != NULL) {
        
        // we write the command in one line and shift the cursor to next line ready for next command
        fprintf(file, "%s\n", cmd);
        
        // close the file once written
        fclose(file);
    } 
    // good to have
    else {
        perror("Error opening history file for writing");
    }
}

// returns the tail of linked list : the most recent command
Node* get_last() { 
    return tail; 
}

// single like condition syntax : 
// (condition) ? <if-true> : <if-false>

// looking at previous pointer of our current node
Node* get_prev(Node *current) {
    return (current && current->prev) ? current->prev : current;
}

// looking at next pointer of our current node
Node* get_next(Node *current) {
    return (current && current->next) ? current->next : current;
}