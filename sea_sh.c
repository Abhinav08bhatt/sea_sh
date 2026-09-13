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