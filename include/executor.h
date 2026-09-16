#ifndef EXECUTOR_H
#define EXECUTOR_H

void execute_command(char *user_input);

#endif

// TODO: Update execute_command() signature to return int (exit status) and take a pointer for elapsed time
// Example signature: int execute_command(char *user_input, double *exec_time);