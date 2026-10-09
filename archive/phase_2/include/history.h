#ifndef HISTORY_H
#define HISTORY_H

typedef struct Node {
    char *command;
    struct Node *prev;
    struct Node *next;
} Node;

void init_history();
void add_history(char *cmd);
Node* get_last();
Node* get_prev(Node *current);
Node* get_next(Node *current);

#endif
