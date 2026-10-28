# sea_sh

> C based command line interpreter

As part of our Project-Based Learning (PBL) for Data Science (Semester 3), this project demonstrates the design and implementation of a custom Command-Line Interpreter developed purely in C language, integrated with predictive history search.

This work was developed under the guidance of our mentor,

### Prof. (Dr.) Varsha Mittal

Presented by:
- Abhinav Bhatt
- Sagar Singh
- Benard Kigongo

---
## PBL phases :

1. ### [Phase 1](archive/phase_1) :
    
    [x] presentation for project proposal

    [x] demo basic shell

2. ### [Phase 2](archive/phase_2) :

    [x] presentation for phase 2

    - #### Feature : 

        [x] implementation of low level shell commands (cd, history, exit, etc)

        [x] using double sided linked list to traverse through the past commands using up/down arrow
        
        [x] implementation of the storage of commands

        [ ] making `tab` suggestion for a command (postponed for phase 3)

        [x] implementation of redirection symbol (`>` and `>>` only)

    - #### Bug handling :

        [x] fixing the ctrl+C os killing the shell

        [x] handling the string in shell inside " " (example during writing a commit message)

3. ### [Phase 3](archive/phase_3/):

    [ ] presentation for phase 3

    - #### Feature :
        
        [ ] making `tab` suggestion for a command (from phase 2)

        [ ] using search algorithm to recommend command from past in form of ghost shell