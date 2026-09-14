# This file contains the explanation of the demo shell code that was presented in the phase 1 of the pbl

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    char user_input[1024];
    char working_directory[1024];

    char *user = getenv("USER");
    if (user == NULL) {
        user = "user";
    }

    while (1) {
    
        getcwd(working_directory, sizeof(working_directory));

        printf("\n%s in %s\n🐚 > ", user, working_directory);
        fflush(stdout);
        
        char *input_status = fgets(user_input, sizeof(user_input), stdin);
        if (input_status == NULL) {
            break;
        }

        user_input[strcspn(user_input, "\n")] = 0;

        if (strcmp(user_input, "exit") == 0) {
            break;
        }

        char *arguments[64];
        int argument_count = 0;
        
        char *token = strtok(user_input, " ");
        while (token != NULL) {
            arguments[argument_count] = token;
            argument_count++;
            token = strtok(NULL, " ");
        }
        
        arguments[argument_count] = NULL;

        if (arguments[0] == NULL) {
            continue;
        } 

        pid_t process_id = fork();

        if (process_id == 0) {
            int execution_status = execvp(arguments[0], arguments);
            
            if (execution_status == -1) {
                perror("Command failed");
            }
            exit(1);

        } else if (process_id > 0) {
            waitpid(process_id, NULL, 0);

        } else {
            perror("Fork failed");
        }
    }

    return 0;
}
```

# Explanation : 

1. ## The Imports
    - `#include <stdio.h>` : for the standard I/O functions
    - `#include <stdlib.h>` : generals utilities like exit(), getenv()
    - `#include <string.h>` : string functions like : strtok(), strcmp(), strcspn()
    - `#include <unistd.h>` : provides us UNIX system calls : fork(), execvp(), getcwd()
    - `#include <sys/wait.h>` : gives us waitpid() so the parent can wait for the child processes

2. ## The Buffers
    ```c
    char user_input[1024];
    char working_directory[1024];
    ```
    - We create a array to store the user input of size 1024 (hoping user will not enter bigger then this)
        - example : whoami, ls, pwd, clear, exit
    - We create a array to store current working dict location of size 1024 (won't be bigger then that is most cases)
        - ~/Code/Collage, ~/Downloads/Folder/File

3. ## Getting User Name
    ```c
    char *user = getenv("USER");
    if (user == NULL) {
        user = "user";
    }
    ```
    - asks the linux system using the getenv function : who is currently using the system? 
        - the returned ans is location of the first element of the user name (and we stores it in array using its pointer)
    - if the username is not found (not possible but still) : we sets the user name to : user (to prevent a crash)

3. ## The Infinite Loop
    ```c
    while (1) {
    
        getcwd(working_directory, sizeof(working_directory));

        printf("%s in %s\n🐚 > ", user, working_directory);
    ```
    - we starts a infinite while loop that makes sure our shell runs forever (until user uses the `exit` command)
    - the `getcwd` function writes the directory location in the variable **working_dict**
    - then we prints the command line prompt with the username, dict location
    ```c
        fflush(stdout);
    ```
    - not needed but helps display prompt instantly without buffer delay

4. ## Taking input
    ```c
        char *input_status = fgets(user_input, sizeof(user_input), stdin);

        if (input_status == NULL) {
            break;
        }

        user_input[strcspn(user_input, "\n")] = 0;
    ```
    - fgets waits and take the input in the variable : input_status (we store the first pointer of the input string array)
    - we check if the input is empty , if yes we exit form the terminal (this can happen when user force kill it using ctrl+d)
    - at the end we remove the newline character "\n" that is stored at the end because of the fgets....using strcspn
    ```c
        if (strcmp(user_input, "exit") == 0) {
            break;
        }
    ```
    - then we check if the user has given the input **exit** if yes we exit from the shell ourselves

5. ## Handling Tokens
    ```c
        char *arguments[64];
        int argument_count = 0;
    ```
    - we create a array variable that will store tokens we separate later on (tokens are arguments given in a command ex : ls -a /Pictures -> here the tokens are "ls", "-a", "/Pictures")
    - and a variable to store the number of tokens (for future access)
    ```c
        char *token = strtok(user_input, " ");
    ```
    - we break the user input into separate tokens using strtok() (this function makes diff elements from a string input with spaces in between : ex : "hello world" -> ["hello","world"]) 
    ```c
        while (token != NULL) {
            arguments[argument_count] = token;
            argument_count++;
            token = strtok(NULL, " ");
        }
    ```
    - once the user input is broken we store inputs in diff elements of the array : for a command like : ls -a /Pictures
        - arguments[0] = "ls"
        - arguments[1] = "-a"
        - arguments[2] = "/Pictures"
    ```c
        arguments[argument_count] = NULL;
    ```
    - we ends the arguments list by the element NULL to indicate the end of the command (execvp needs it for clarification)
    ```c
        if (arguments[0] == NULL) {
            continue;
        }
    ```
    - if user inputs "enter" simply without writing anything the shell must loops itself and show the prompt again

6. ## Process Creation and Execution
    
    we run the command using the execvp() function, but execvp runs a command and kills the shell after that....
    so we create a clone command which runs while the execvp commands wait and replaces itself with the clone command output

    ```c
        pid_t process_id = fork();
    ```
    - we create a clone terminal (hidden)
    - forks returns 2 diff values :
        - "0" to the child command 
        - "a number greater then 0" (PDI : Process ID) to the parent command
    - using this both commands know who they are
    - IF FORK FAILED (can happen when system out of memory) : it returns -1

    ```c
        if (process_id == 0) {
            int execution_status = execvp(arguments[0], arguments);
    ```
    > only the child process (clone shell) will execute the following
    - we get the process ID 0 for a child command (in the clone hidden shell)
    - When we get the child command we execute it : now the child process is not running our code anymore but is running the command code
    - if the execvp succeeds, it will never return back (child command will run, show the output and kill itself)
    ```c
            if (execution_status == -1) {
                perror("Command failed");
            }
    ```
    - if the command failed (we input a fake command like : "quwefb" that does not exist in system)
    ```c
            exit(1);
        }
    ```
    - if failed we need to exit from the child command (or it will keep running forever in the clone terminal)
    ```c
        else if (process_id > 0) {
            waitpid(process_id, NULL, 0);
        }
    ```
    > only the parent process (our shell) will run the following
    - waitpid() makes the parent process sleep while the child process is running
    - (if this is not written the parent process will show the next prompt before the child finishes its process)
    ```c
        else {
            perror("Fork failed");
        }
    ```
    > if the system failed to create a clone process when system is out of memory

```cpp
[ Parent Shell ] 
              │
        fork() called
        ┌─────┴────────────────┐
        ▼                      ▼
  [ Child (PID = 0) ]    [ Parent (PID > 0) ]
        │                      │
  execvp("ls")           waitpid(...)
  (Transforms into ls)   (Pauses and waits...)
        │                      │
   Prints files                │
        │                      │
   ls Finishes & Dies          │
        └─────────────────────►│
                         Child is done!
                         Parent wakes up.
                         Prints next prompt: 🐚 >
```