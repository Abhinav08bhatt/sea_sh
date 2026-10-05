# Phase 2: The Interactive Shell

Welcome to Phase 2 of `sea_sh`! 

In Phase 1, we used simple C functions like `fgets()` to read a whole line of text at once. It worked, but it was limited. We couldn't use the Up arrow to see previous commands, and we couldn't use the Left/Right arrows to edit our text. 

To fix this, we had to completely take control of the terminal. Instead of waiting for the user to press "Enter", we now read **every single keypress as it happens**. 

Here is the step-by-step, simple English explanation of how our Phase 2 code works.

---

## 1. Taking Control: Raw Mode (`terminal.c`)

Normally, the terminal is in "Cooked Mode". This means it automatically shows the keys you type, handles backspace for you, and only sends the text to our C program when you press Enter. We don't want that anymore.

    void enable_raw_mode() {
        tcgetattr(STDIN_FILENO, &original_termios);
        struct termios raw = original_termios;

        // Turn off ECHO and ICANON
        raw.c_lflag &= ~(ECHO | ICANON);

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

### What is this doing?
* **`tcgetattr`**: We ask the system for the current terminal settings and save them so we can restore them later.
* **`ECHO`**: We turn this **OFF**. This means when you type the letter 'A', the terminal won't automatically print 'A' on the screen. We have to print it manually!
* **`ICANON`**: We turn this **OFF**. This stops the terminal from waiting for you to press "Enter". Now, the moment any key is pressed, our program gets it instantly.

---

## 2. Reading Keys One-by-One (`sea_sh.c`)

Inside our `main()` function, we have an infinite `while(1)` loop. This shows our prompt and starts reading your keyboard.

        int i = 0;
        int cursor_pos = 0;
        char c;
        user_input[0] = '\0';
        
        while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n') {
            // ... handling characters ...
        }

### What is this doing?
* **`i`**: Keeps track of how many characters are in our command string.
* **`cursor_pos`**: Keeps track of where our visual cursor is (we need this so we can move left and right).
* **`user_input[0] = '\0'`**: We clear out the buffer to make sure it's clean for the new command.
* **`read(STDIN_FILENO, &c, 1)`**: This reads exactly **1 character at a time**. It only stops looping when you press `\n` (the Enter key).

---

## 3. Handling Arrow Keys (Left & Right)

When you press an arrow key, your keyboard sends a hidden "Escape Sequence"—a combo of 3 characters:
1. `ESC` (Number 27 in ASCII)
2. `[` (Number 91 in ASCII)
3. `C` (Right) or `D` (Left)

            if (c == 27) {
                read(STDIN_FILENO, &c, 1); // reads '['
                read(STDIN_FILENO, &c, 1); // reads 'C' or 'D'
                
                if (c == 'C') { // Right Arrow
                    if (cursor_pos < i) {
                        printf("\033[C");
                        cursor_pos++;
                    }
                } 
                else if (c == 'D') { // Left Arrow
                    if (cursor_pos > 0) {
                        printf("\033[D");
                        cursor_pos--;
                    }
                }
            }

### What is this doing?
* We catch the `27` (Escape) and read the next two characters.
* If it's **Right (`C`)** or **Left (`D`)**, we check if we actually have room to move. If we do, we print `\033[C` or `\033[D` (which literally moves the terminal cursor visually) and update our internal `cursor_pos` tracker.

---

## 4. Time Travel: History & Linked Lists (`history.c`)

To make the **Up** and **Down** arrows work, we need to remember past commands. For this, we use a **Doubly Linked List**. 

If you are new to Linked Lists, think of it like a train. Every train car (a "Node") holds a command, and it knows which car is in front of it (`prev`) and which car is behind it (`next`).

    typedef struct Node {
        char *command;
        struct Node *prev;
        struct Node *next;
    } Node;

When you press the **Up Arrow (`A`)**:
* If we are at the bottom, we look at the last command (`get_last()`).
* If we press Up again, we move to the previous train car (`get_prev()`).

When you press the **Down Arrow (`B`)**:
* We move to the next train car (`get_next()`).

After finding the right command from history, we copy it into `user_input` and print it on the screen so you can edit it!

---

## 5. Typing and Backspace Magic (`sea_sh.c`)

Because we turned off `ECHO` and `ICANON`, we have to handle the Backspace key (ASCII 127) and normal typing all by ourselves.

### Normal Typing:
If you move your cursor to the middle of a word and type a letter, we can't just put it at the end. We have to "shift" all the letters to the right to make a hole, put the new letter in the hole, and then print the rest of the string so it looks normal on the screen.

    memmove(&user_input[cursor_pos + 1], &user_input[cursor_pos], i - cursor_pos + 1);
    user_input[cursor_pos++] = c;
    i++;

### Backspace (127):
If you press backspace, we do the opposite. We take all the text on the right side of the cursor and "shift" it one step to the left, overwriting the character you just deleted.

    memmove(&user_input[cursor_pos - 1], &user_input[cursor_pos], i - cursor_pos + 1);
    i--;
    cursor_pos--;

---

## 6. Aesthetics: The Prompt (`prompt.c`)

*(Don't stress too much about this file, it just makes things look pretty!)*

Before asking for input, we call `show_prompt()`. This function gathers:
1. Your username (`getenv("USER")`).
2. Your current folder (`getcwd()`).
3. An OS Icon (Apple logo for Mac, Linux Penguin for Linux).
4. Git branch (If you are in a coding folder, it runs `git rev-parse` to find the branch name).
5. Execution Time (How long your last command took to run).

It prints all this out in pretty colors using ANSI color codes (like `\033[32m` for Green).

---

## 7. The Engine: Executing Commands (`executor.c`)

Once you press "Enter", the `while` loop finishes. We add your command to our history train, and send the string to `execute_command()`.

    void execute_command(char *user_input) {
        // Break input into tokens
        char *token = strtok(user_input, " ");
        while (token != NULL) {
            arguments[argument_count] = token;
            argument_count++;
            token = strtok(NULL, " ");
        }
        arguments[argument_count] = NULL;
    }

### What is this doing?
* **`strtok`**: This chops the string into pieces. If you type `"sudo apt update"`, it changes the spaces to `\0` (NULL characters) and gives us an array: `["sudo", "apt", "update", NULL]`.

### Handling Built-ins (`builtins.c`):
Next, we check if the command is a "Built-in" command. 

    if (handle_builtins(arguments) == 1) {
        return; // Handled! Go back to main loop.
    }

* Commands like `cd` (Change Directory) and `exit` don't exist as separate programs on your computer. They have to be executed *by the shell itself*.
* If the user typed `cd`, we use the C function `chdir()` to change the folder.
* If the user typed `exit`, we call `exit(0)` to close the shell.

### Executing System Commands (The Fork):
If the command was NOT a built-in (like `ls`, `cat`, `python`), we ask the operating system to run it.

    pid_t process_id = fork();

    if (process_id == 0) {
        execvp(arguments[0], arguments);
        exit(1);
    } else if (process_id > 0) {
        waitpid(process_id, NULL, 0);
    }

* **`fork()`**: This creates a clone of our shell.
* **Child (process_id == 0)**: The clone uses `execvp()` to transform itself into the command (like `ls`). When it finishes printing the files, it dies.
* **Parent (process_id > 0)**: Our actual shell uses `waitpid()` to pause and wait. Once the child is dead, we wake up and show the prompt again!

---

**And that's it!** That is the complete flow of Phase 2. We read characters manually, handle visual cursor movement, save history in a linked list, format a beautiful prompt, and run commands through the OS.