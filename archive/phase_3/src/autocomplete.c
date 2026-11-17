// this file provides ghost autocomplete suggestions from history and common commands

#include <stddef.h>
#include <string.h>

#include "../include/autocomplete.h"
#include "../include/history.h"

// fallback list of common commands (used if nothing in history matches yet)
static const char *common_commands[] = {
    "ls -la",
    "cd ",
    "cat ",
    "echo ",
    "exit",
    "grep ",
    "mkdir ",
    "touch ",
    "rm ",
    "cp ",
    "mv ",
    "pwd",
    "clear",
    "sort ",
    "wc -l",
    "head ",
    "tail ",
    "make",
    "git status",
    "git add ",
    "git commit -m \"\"",
    "git push",
    "git pull",
    NULL
};

// searches for a command that starts with 'prefix' and returns ONLY the remaining part (suffix)
// example : prefix = "git st", match = "git status" -> returns "atus"
const char* get_suggestion(const char *prefix) {

    // if user has typed nothing yet, don't show any ghost suggestion
    if (prefix == NULL || prefix[0] == '\0') {
        return NULL;
    }

    size_t len = strlen(prefix);

    // 1. Search history from MOST RECENT (tail) backwards to oldest (head)
    // (we use curr->prev directly so it stops cleanly at NULL)
    Node *curr = get_last();
    while (curr != NULL) {
        if (curr->command != NULL &&
            strncmp(curr->command, prefix, len) == 0 &&
            strlen(curr->command) > len) {
            // return pointer to the part AFTER what the user already typed
            return curr->command + len;
        }
        curr = curr->prev;
    }

    // 2. If history had no match, check our small fallback list of common commands
    for (int i = 0; common_commands[i] != NULL; i++) {
        if (strncmp(common_commands[i], prefix, len) == 0 &&
            strlen(common_commands[i]) > len) {
            return common_commands[i] + len;
        }
    }

    // no match found
    return NULL;
}