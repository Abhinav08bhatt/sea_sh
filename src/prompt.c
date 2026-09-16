#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/prompt.h"

#define COLOR_RESET "\033[0m"
#define COLOR_BOLD "\033[1m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"

#define CURSOR_BLINKING "\033[5 q"
#define CURSOR_BLINKING "\033[5 q"
#define UNDERLINE_BLINKING "\033[3 q"

int get_git_branch(char *branch,size_t size) {

    FILE *file_pointer = popen("git rev-parse --abbrev-ref HEAD 2>/dev/null","r");
    if(file_pointer == NULL){
        return 0;
    }

    if (fgets(branch, size, file_pointer) != NULL){
        branch[strcspn(branch, "\n")] = '\0';
        pclose(file_pointer);
        return (strlen(branch)>0);
    }

    pclose(file_pointer);
    return 0;
}

const char *get_os_icon(void) {
    #if defined(__APPLE__)
        return "  ";
    #elif defined(__linux__)
        return "  ";
    #else
        return " 💻 ";
    #endif
}

void show_prompt(){
    
    char working_directory[1024];

    char *user = getenv("USER");
    if (user==NULL){
        user = "user";
    }

    getcwd(working_directory, sizeof(working_directory));
    
    char git_branch[256];
    
    printf("\n" 
        COLOR_BOLD "%s" COLOR_RESET 
        COLOR_CYAN "%s" COLOR_RESET 
        " in " 
        COLOR_MAGENTA "%s" COLOR_RESET ,
        get_os_icon() ,
        user, 
        working_directory
    );

    if (get_git_branch(git_branch, sizeof(git_branch))) {
        printf(" on " 
            COLOR_GREEN " %s" 
            COLOR_RESET,
            git_branch
        );
    }

    printf("\n 🐚 ❯ "UNDERLINE_BLINKING);

    fflush(stdout);
}


// TODO: 1. Add ANSI color macros for Success (Cyan) and Error (Red).
// TODO: 2. Parse advanced Git status: ahead (⇡), behind (⇣), staged (+), modified (!), untracked (?).
// TODO: 3. Print execution time if command took time (e.g., "took 3.24s").
// TODO: 4. Dynamically color the '❯' prompt character (Cyan on success 0, Red on error != 0).