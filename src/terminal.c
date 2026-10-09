// this file is used to handle the terminal settings and enable raw mode for the shell

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// the library that helps us to activate raw terminal mode
#include <termios.h>

// this structure holds the original terminal settings
struct termios original_termios;

// function to disable raw mode and restore the original terminal settings
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// function to enable the raw mode of the terminal
// raw mode means : 
void enable_raw_mode() {

    // terminal control get attributes : it asks the system for "standard input" (STDIN_FILENO)
    // and saved them into our structure "original_termios"
    tcgetattr(STDIN_FILENO, &original_termios);
    //! STDIN_FILENO: is a file indicator (in linux everything is treaded as a file input,ouput,erro) 
    // 0 : standard input
    // 1 : standard output
    // 2 : standard error


    // if our program finishes for any reason (crash) we run disable_raw_mode function automatically
    // if we dont the terminal might get blank
    //! commenting it rn as it is being unpredictable
    // atexit(disable_raw_mode);

    // we create a copy of the original terminal settings so we can modify it to enable raw mode
    struct termios raw = original_termios;

    // here we are modifying two terminal settings : ECHO and ICANON
    // ECHO : a builtin function of terminal that shows every key we press in terminal
    // ICANON : it is cooking mode : we turn it off : now we dont need to wait for user to press "enter" to read the input : we read the input every miliseconds
    // the terminal settings :"c_lflag": are present in bitmask (series of 0 and 1)
    // for modification we just invert the value of the bitmask of a given setting
    // meaning if ECHO was 1 ... we flip it to 0 (turning it off)
    // meaning if ICANON was 1 ... we flip it to 0 (turning it off)
    raw.c_lflag &= ~(ECHO | ICANON);

    // saving the new terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}