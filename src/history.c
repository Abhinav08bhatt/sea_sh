#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/history.h"

static Node *head = NULL;
static Node *tail = NULL;

// this function adds a command to the history linked list
// it creates a new node in simple language and adds it to the end of the linked list
void add_history(char *cmd) {
    if (strlen(cmd) == 0) return;
    Node *new_node = malloc(sizeof(Node));
    new_node->command = strdup(cmd);
    new_node->next = NULL;
    new_node->prev = tail;

    if (tail) {
        tail->next = new_node;
    } else {
        head = new_node;
    }
    tail = new_node;
}

// returns the tail of linked list : the most recent command
Node* get_last() { 
    return tail; 
}

// looking at previous pointer of our current node
Node* get_prev(Node *current) {
    return (current && current->prev) ? current->prev : current;
}

// looking at next pointer of our current node
Node* get_next(Node *current) {
    return (current && current->next) ? current->next : current;
}