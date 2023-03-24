#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tc.h"
#include "list.h"

// Lexical analysis of source code and return a list of tokens
struct list_head* lex(char* source_code)
{
    struct list_head* tokens_list = malloc(sizeof(struct list_head));
    INIT_LIST_HEAD(tokens_list);

    char* current_char = source_code;

    while (*current_char != '\0') {
        // Skip whitespace
        while (isspace(*current_char)) {
            current_char++;
        }

        // Skip comments
        if (*current_char == '/') {
	    //Skip single line comments
            current_char++;
            if (*current_char == '/') {
                current_char++;
                while (*current_char != '\n') {
                    current_char++;
                }
                continue;
            } else if (*current_char == '*') {
                //Skip multi line comments
                current_char++;
                while (*current_char != '*' || *(current_char+1) != '/') {
                    current_char++;
                }
                current_char += 2;
                continue;
            } else {
                current_char--;
            }
        }

        // Parse identifiers and keywords
        if (isalpha(*current_char) || *current_char == '_') {
        // TODO: Implement identifier and keyword parsing
        }

        // Parse numbers
        if (isdigit(*current_char)) {
            // TODO: Implement number parsing
        }

        // Parse strings and characters
        if (*current_char == '\"' || *current_char == '\'') {
            // TODO: Implement string and character parsing
        }

        // Parse operators
        if (*current_char == '+' || *current_char == '-' || *current_char == '*' ||
            *current_char == '/' || *current_char == '%' || *current_char == '!' ||
            *current_char == '&' || *current_char == '|' || *current_char == '^' ||
            *current_char == '~' || *current_char == '<' || *current_char == '>' ||
            *current_char == '=' || *current_char == '.') {
            // TODO: Implement operator parsing
        }

        // Parse separators
        if (*current_char == ',' || *current_char == ';' || *current_char == ':' ||
            *current_char == '(' || *current_char == ')' || *current_char == '[' ||
            *current_char == ']' || *current_char == '{' || *current_char == '}') {
            // TODO: Implement separator parsing
        }

        current_char++;
    }

    return tokens_list;
}
