// this file is responsible for showing the prompt to the user

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/prompt.h"

// creating macros for colors and cursor styles
// colors
#define COLOR_RESET "\033[0m"
#define COLOR_BOLD "\033[1m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
// cursor
#define CURSOR_BLINKING "\033[5 q"
#define CURSOR_BLINKING "\033[5 q"
#define UNDERLINE_BLINKING "\033[3 q"

// reads the git information
int get_git_branch(char *branch,size_t size) {

    // reading the git branch name
    // to test this command run : git rev-parse --abbrev-ref HEAD
    FILE *file_pointer = popen("git rev-parse --abbrev-ref HEAD 2>/dev/null","r");
    
    // if not git branch is found we return accepting there never was a git repo
    if(file_pointer == NULL){
        return 0;
    }

    // reading the branch name from the file pointer and removing the newline character
    // fgets reads in a buffer named as branch which we passed to the function and defined in main show_prompt function
    // we give the size of the buffer to fgets so that it does not overflow (we defined the size of the buffer in main show_prompt function)
    // reading the file_pointer that has our git branch name rn
    // we are checking if fgets reads the branch name cleanly (it returns NULL if it hits the end of the file or an error occurs)
    if (fgets(branch, size, file_pointer) != NULL){

        // removing the newline character from the branch name
        branch[strcspn(branch, "\n")] = '\0';

        // closing the file pointer (keeping stuff clean)
        pclose(file_pointer);

        // returning true if the branch name is not empty (returns 1)
        return (strlen(branch)>0);
    }

    // closing the file pointer if the upper if condition fails
    pclose(file_pointer);
    return 0;
}

// returns the OS icon based on the operating system
const char *get_os_icon(void) {
    #if defined(__APPLE__)
        return "  ";
    #elif defined(__linux__)
        return "  ";
    #else
        return " 💻 ";
    #endif
}

// this function is responsible for showing the prompt to the user
void show_prompt(double time_taken){
    
    // getting the current working directory
    char working_directory[1024];

    // getting the user name
    char *user = getenv("USER");
    // if could not get the user name we set it to "user" (default)
    if (user==NULL){
        user = "user";
    }

    // getting the current working directory and storing it in the working_directory variable
    getcwd(working_directory, sizeof(working_directory));
    
    // creating a variable to store the git branch name
    char git_branch[256];
    
    // printing the prompt
    // (os_icon) (user) (working_dict)
    printf( ""
        COLOR_BOLD "%s" COLOR_RESET 
        COLOR_CYAN "%s" COLOR_RESET 
        " in " 
        COLOR_MAGENTA "%s" COLOR_RESET ,
        get_os_icon() ,
        user, 
        working_directory
    );

    // adding the git branch and time taken to prompt
    // of the git branch is found this function returns TRUE and condition works
    if (get_git_branch(git_branch, sizeof(git_branch))) {

        // if time is too small we dont show it
        if (time_taken < 0.01){
            printf(" on " 
                COLOR_GREEN " %s" 
                COLOR_RESET,
                git_branch
            );
        }
        // if time is significant we show it
        else {
            printf(" on " 
                COLOR_GREEN " %s" 
                COLOR_RESET " took %.2f",
                git_branch,time_taken
            );
        }
    }
    // if time is significant we show it (even if git branch is not found)
    else if (time_taken >= 0.01){
        printf(" took %.2f",time_taken);
    }

    // showing the prompt character with blinking underline cursor
    printf("\n 🐚 ❯ "UNDERLINE_BLINKING);

    // flushing the output buffer to ensure the prompt is displayed immediately (to keep stuff clean (not necessary))
    fflush(stdout);
}


// TODO: 1. Add ANSI color macros for Success (Cyan) and Error (Red).
// not done yet
// TODO: 2. Parse advanced Git status: ahead (⇡), behind (⇣), staged (+), modified (!), untracked (?).
// not done yet
// TODO: 3. Print execution time if command took time (e.g., "took 3.24s").
//? DONE
// TODO: 4. Dynamically color the '❯' prompt character (Cyan on success 0, Red on error != 0).
// not done yet (same as TODO 1)