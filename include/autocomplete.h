#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

// given what user has typed so far (e.g. "git st"),
// returns the remaining ghost text suffix (e.g. "atus"), or NULL if no match
const char* get_suggestion(const char *prefix);

#endif