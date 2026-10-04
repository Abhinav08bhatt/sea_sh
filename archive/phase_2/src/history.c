typedef struct Node {
    char *command;
    struct Node *prev;
    struct Node *next;
} Node;