#define _GNU_SOURCE // For strndup
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tc.h"
#include "list.h"

// Parse separator and reutrn the length of the separator
static size_t parse_separator(token_t* token, char* current_char)
{
    switch (*current_char) {
        case ',':
            token->type = TOK_SEPARATOR_COMMA;
            break;
        case ';':
            token->type = TOK_SEPARATOR_SEMICOLON;
            break;
        case ':':
            token->type = TOK_SEPARATOR_COLON;
            break;
        case '(':
            token->type = TOK_SEPARATOR_LEFT_PARENTHESIS;
            break;
        case ')':
            token->type = TOK_SEPARATOR_RIGHT_PARENTHESIS;
            break;
        case '[':
            token->type = TOK_SEPARATOR_LEFT_BRACKET;
            break;
        case ']':
            token->type = TOK_SEPARATOR_RIGHT_BRACKET;
            break;
        case '{':
            token->type = TOK_SEPARATOR_LEFT_BRACE;
            break;
        case '}':
            token->type = TOK_SEPARATOR_RIGHT_BRACE;
            break;
        default:
            break;
    }
    token->lexeme = strndup(current_char, 1);
    tc_debug(0, "%s\n", token->lexeme);
    return 1;
}

// Parse operator and reutrn the length of the operator
static size_t parse_operator(token_t* token, char* current_char)
{
    char *start = current_char;
    switch (*current_char) {
        case '+':
            if (*(current_char + 1) == '+') {
                // ++ operator
                token->type = TOK_OPERATOR_INC;
                current_char++;
            } else if (*(current_char + 1) == '=') {
                // += operator
                token->type = TOK_OPERATOR_ADD_ASSIGN;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_ADD;
            }
            break;
        case '-':
            if (*(current_char + 1) == '-') {
                // -- operator
                token->type = TOK_OPERATOR_DEC;
                current_char++;
            } else if (*(current_char + 1) == '=') {
                // -= operator
                token->type = TOK_OPERATOR_SUB_ASSIGN;
                current_char++;
            } else if (*(current_char + 1) == '>') {
                // -> operator
                token->type = TOK_OPERATOR_DEREFERENCE;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_SUB;
            }
            break;
        case '*':
            if (*(current_char + 1) == '=') {
                // *= operator
                token->type = TOK_OPERATOR_MUL_ASSIGN;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_MUL;
            }
            break;
        case '/':
            if (*(current_char + 1) == '=') {
                // /= operator
                token->type = TOK_OPERATOR_DIV_ASSIGN;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_DIV;
            }
            break;
        case '=':
            // == operator
            if (*(current_char + 1) == '=') {
                token->type = TOK_OPERATOR_EQUAL;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_ASSIGNMENT;
            }
            break;
        case '%':
            // %= operator
            if (*(current_char + 1) == '=') {
                token->type = TOK_OPERATOR_MOD_ASSIGN;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_MOD;
            }
            break;
        case '!':
            if (*(current_char + 1) == '=') {
                // != operator
                token->type = TOK_OPERATOR_NOT_EQUAL;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_LOGICAL_NOT;
            }
            break;
        case '&':
            if (*(current_char + 1) == '&') {
                // && operator
                token->type = TOK_OPERATOR_LOGICAL_AND;
                current_char++;
            } else if (*(current_char + 1) == '=') {
                // &= operator
                token->type = TOK_OPERATOR_BITWISE_AND_ASSIGN;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_BITWISE_AND;
            }
            break;
        case '|':
            if (*(current_char + 1) == '|') {
                // || operator
                token->type = TOK_OPERATOR_LOGICAL_OR;
                current_char++;
            } else if (*(current_char + 1) == '=') {
                // |= operator
                token->type = TOK_OPERATOR_BITWISE_OR_ASSIGN;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_BITWISE_OR;
            }
            break;
        case '.':
            if (*(current_char + 1) == '.') {
                // .. operator
                token->type = TOK_OPERATOR_RANGE;
                current_char++;
            } else {
                token->type = TOK_OPERATOR_DOT;
            }
            break;
        // TODO: add more operators
            panic("Not implemented");
        default:
            break;
    }
    current_char++;
    token->lexeme = strndup(start, current_char - start);
    tc_debug(0, "%s\n", token->lexeme);

    return current_char - start;
}

// Parse string or character literal and reutrn the length of the literal
static size_t parse_string_or_char(token_t* token, char* current_char)
{
    char* start = current_char;

   if (*current_char == '\"') {
       // String literal (e.g. "Hello, world!")
        current_char++;
        while (*current_char != '\"') {
            if (*current_char == '\\') {
                //Skip escaped next character
                current_char++;
            }
            current_char++;
        }
        current_char++;
        token->type = TOK_CONSTANT_STRING;
    } else if (*current_char == '\'') {
       // Character literal (e.g. 'a')j
        current_char++;
        while (*current_char != '\'') {
            if (*current_char == '\\') {
                //Skip escaped next character
                current_char++;
            }
            current_char++;
        }
        current_char++;
        token->type = TOK_CONSTANT_CHAR;
    }

    token->lexeme = strndup(start, current_char - start);
    return current_char - start;
}

// Parse a number and return the length of the number
// TODO: Handle binary, octal and hexadecimal numbers
static size_t parse_number(token_t* token, char* current_char)
{
    char* start = current_char;

    // Parse the integer part of the number
    while (isdigit(*current_char)) {
        current_char++;
    }

    // Parse the decimal part of the number, if present
    if (*current_char == '.') {
        current_char++;
        while (isdigit(*current_char)) {
            current_char++;
        }

        // Create a token for the floating-point constant
        token->type = TOK_CONSTANT_FLOAT;
    } else if (*current_char == 'l' || *current_char == 'L') {
        current_char++;
        // Create a token for the long integer constant
        token->type = TOK_CONSTANT_LONG;
    } else {
        // Create a token for the integer constant
        token->type = TOK_CONSTANT_INT;
    }
    token->lexeme = strndup(start, current_char - start);
    tc_debug(0, "%s\n", token->lexeme);

    return current_char - start;
}

// Parse a keyword or identifier and return the length of the keyword or identifier
// TODO: Use a hash table to speed up the lookup
static size_t parse_keyword_or_id(token_t* token, char* current_char)
{
    size_t len = 1;
    while (isalnum(*(current_char + len)) || *(current_char + len) == '_') {
        len++;
    }
    char* str = strndup(current_char, len);
    tc_debug(0, "%s\n", str);

    if (!strcmp(str, "auto")) {
        token->type = TOK_KEYWORD_AUTO;
    } else if (!strcmp(str, "break")) {
        token->type = TOK_KEYWORD_BREAK;
    } else if (!strcmp(str, "case")) {
        token->type = TOK_KEYWORD_CASE;
    } else if (!strcmp(str, "char")) {
        token->type = TOK_KEYWORD_CHAR;
    } else if (!strcmp(str, "const")) {
        token->type = TOK_KEYWORD_CONST;
    } else if (!strcmp(str, "continue")) {
        token->type = TOK_KEYWORD_CONTINUE;
    } else if (!strcmp(str, "default")) {
        token->type = TOK_KEYWORD_DEFAULT;
    } else if (!strcmp(str, "do")) {
        token->type = TOK_KEYWORD_DO;
    } else if (!strcmp(str, "double")) {
        token->type = TOK_KEYWORD_DOUBLE;
    } else if (!strcmp(str, "else")) {
        token->type = TOK_KEYWORD_ELSE;
    } else if (!strcmp(str, "enum")) {
        token->type = TOK_KEYWORD_ENUM;
    } else if (!strcmp(str, "extern")) {
        token->type = TOK_KEYWORD_EXTERN;
    } else if (!strcmp(str, "float")) {
        token->type = TOK_KEYWORD_FLOAT;
    } else if (!strcmp(str, "for")) {
        token->type = TOK_KEYWORD_FOR;
    } else if (!strcmp(str, "goto")) {
        token->type = TOK_KEYWORD_GOTO;
    } else if (!strcmp(str, "if")) {
        token->type = TOK_KEYWORD_IF;
    } else if (!strcmp(str, "inline")) {
        token->type = TOK_KEYWORD_INLINE;
    } else if (!strcmp(str, "int")) {
        token->type = TOK_KEYWORD_INT;
    } else if (!strcmp(str, "long")) {
        token->type = TOK_KEYWORD_LONG;
    } else if (!strcmp(str, "register")) {
        token->type = TOK_KEYWORD_REGISTER;
    } else if (!strcmp(str, "return")) {
        token->type = TOK_KEYWORD_RETURN;
    } else if (!strcmp(str, "short")) {
        token->type = TOK_KEYWORD_SHORT;
    } else if (!strcmp(str, "signed")) {
        token->type = TOK_KEYWORD_SIGNED;
    } else if (!strcmp(str, "sizeof")) {
        token->type = TOK_KEYWORD_SIZEOF;
    } else if (!strcmp(str, "static")) {
        token->type = TOK_KEYWORD_STATIC;
    } else if (!strcmp(str, "struct")) {
        token->type = TOK_KEYWORD_STRUCT;
    } else if (!strcmp(str, "switch")) {
        token->type = TOK_KEYWORD_SWITCH;
    } else if (!strcmp(str, "typedef")) {
        token->type = TOK_KEYWORD_TYPEDEF;
    } else if (!strcmp(str, "union")) {
        token->type = TOK_KEYWORD_UNION;
    } else if (!strcmp(str, "unsigned")) {
        token->type = TOK_KEYWORD_UNSIGNED;
    } else if (!strcmp(str, "void")) {
        token->type = TOK_KEYWORD_VOID;
    } else if (!strcmp(str, "volatile")) {
        token->type = TOK_KEYWORD_VOLATILE;
    } else if (!strcmp(str, "while")) {
        token->type = TOK_KEYWORD_WHILE;
    } else {
        token->type = TOK_IDENTIFIER;
    }
    token->lexeme = str;
    return len;
}

// Lexical analysis of source code and return a list of tokens
struct list_head* lex(char* source_code)
{
    struct list_head* tokens_list = malloc(sizeof(struct list_head));
    INIT_LIST_HEAD(tokens_list);

    char* current_char = source_code;

    tc_debug(0, "\n%s\n", source_code);
    while (*current_char != '\0') {
        // Skip whitespace
        if (isspace(*current_char)) {
            current_char++;
            continue;
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
                current_char--; //Not a comment, so go back and fall through
            }
        }

        // Parse identifiers and keywords
        if (isalpha(*current_char) || *current_char == '_') {
            token_t* token = malloc(sizeof(token_t));
            current_char += parse_keyword_or_id(token, current_char);
            list_add_tail(&token->list, tokens_list);
            continue;
        }

        // Parse numbers
        if (isdigit(*current_char)) {
            token_t* token = malloc(sizeof(token_t));
            current_char += parse_number(token, current_char);
            list_add_tail(&token->list, tokens_list);
            continue;
        }


        // Parse strings and characters
        if (*current_char == '\"' || *current_char == '\'') {
            token_t* token = malloc(sizeof(token_t));
            current_char += parse_string_or_char(token, current_char);
            list_add_tail(&token->list, tokens_list);
            continue;
        }

        // Parse operators
        if (*current_char == '+' || *current_char == '-' || *current_char == '*' ||
            *current_char == '/' || *current_char == '%' || *current_char == '!' ||
            *current_char == '&' || *current_char == '|' || *current_char == '^' ||
            *current_char == '~' || *current_char == '<' || *current_char == '>' ||
            *current_char == '=' || *current_char == '.') {
            token_t* token = malloc(sizeof(token_t));
            current_char += parse_operator(token, current_char);
            list_add_tail(&token->list, tokens_list);
            continue;
        }

        // Parse separators
        if (*current_char == ',' || *current_char == ';' || *current_char == ':' ||
            *current_char == '(' || *current_char == ')' || *current_char == '[' ||
            *current_char == ']' || *current_char == '{' || *current_char == '}') {
            token_t* token = malloc(sizeof(token_t));
            current_char += parse_separator(token, current_char);
            list_add_tail(&token->list, tokens_list);
            continue;
        }

        panic("Unknown character: [%c:%d]\n", *current_char, *current_char);
    }

    #ifdef TC_DEBUG
    token_t *t;
    list_for_each_entry(t, tokens_list, list) {
        tc_debug(1, "[%s:%d]\n", t->lexeme, t->type);
    }
    #endif

    return tokens_list;
}
