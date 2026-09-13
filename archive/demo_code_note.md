# This file contains the explanation of the demo shell code that was presented in the phase 1 of the pbl

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    char input[1024];
    char cwd[1024];

    char *user = getenv("USER");
    if (user == NULL) user = "user";

    while (1) {
        getcwd(cwd, sizeof(cwd));

        printf("\n%s in %s\n🐚 > ", user, cwd);
        if (!fgets(input, sizeof(input), stdin)) break;

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) break;

        char *args[64];
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue;

        pid_t pid = fork();

        if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("Command failed");
            }
            exit(1);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0);
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
    ```
    - fgets waits and take the input