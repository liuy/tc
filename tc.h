#ifndef TC_H
#define TC_H

#include <stdio.h>
#include <stdarg.h>
#include "list.h"

typedef struct ast_node {
    // TODO: Define AST node structure
} ast_node_t;

typedef struct symbol_table {
    // TODO: Define symbol table structure
} symbol_table_t;

typedef struct code_generator {
    // TODO: Define code generator structure
} code_generator_t;

typedef struct optimization_pass {
    // TODO: Define optimization pass structure
} optimization_pass_t;

typedef struct debug_info {
    // TODO: Define debug info structure
} debug_info_t;

typedef struct token {
    struct list_node list;
    char *lexeme;
    int type;
} token_t;

enum token_type {
    TOK_KEYWORD_AUTO, // auto
    TOK_KEYWORD_BREAK, // break
    TOK_KEYWORD_CASE, // case
    TOK_KEYWORD_CHAR, // char
    TOK_KEYWORD_CONST, // const
    TOK_KEYWORD_CONTINUE, // continue
    TOK_KEYWORD_DEFAULT, // default
    TOK_KEYWORD_DO, // do
    TOK_KEYWORD_DOUBLE, // double
    TOK_KEYWORD_ELSE, // else
    TOK_KEYWORD_ENUM, // enum
    TOK_KEYWORD_EXTERN, // extern
    TOK_KEYWORD_FLOAT, // float
    TOK_KEYWORD_FOR, // for
    TOK_KEYWORD_GOTO, // goto
    TOK_KEYWORD_IF, // if
    TOK_KEYWORD_INT, // int
    TOK_KEYWORD_LONG, // long
    TOK_KEYWORD_REGISTER, // register
    TOK_KEYWORD_RETURN, // return
    TOK_KEYWORD_SHORT, // short
    TOK_KEYWORD_SIGNED, // signed
    TOK_KEYWORD_SIZEOF, // sizeof
    TOK_KEYWORD_STATIC, // static
    TOK_KEYWORD_STRUCT, // struct
    TOK_KEYWORD_SWITCH, // switch
    TOK_KEYWORD_TYPEDEF, // typedef
    TOK_KEYWORD_UNION, // union
    TOK_KEYWORD_UNSIGNED, // unsigned
    TOK_KEYWORD_VOID, // void
    TOK_KEYWORD_VOLATILE, // volatile
    TOK_KEYWORD_WHILE, // while
    TOK_KEYWORD_INLINE, // inline
    TOK_IDENTIFIER, // Identifier
    TOK_CONSTANT_INT, // Integer constant
    TOK_CONSTANT_FLOAT, // Floating-point constant
    TOK_CONSTANT_LONG, // Long Integer constant
    TOK_CONSTANT_CHAR, // Character constant
    TOK_CONSTANT_STRING, // String constant
    TOK_OPERATOR_ADD, // "+"
    TOK_OPERATOR_SUB, // "-"
    TOK_OPERATOR_MUL, // "*"
    TOK_OPERATOR_DIV, // "/"
    TOK_OPERATOR_MOD, // "%"
    TOK_OPERATOR_INC, // "++"
    TOK_OPERATOR_DEC, // "--"
    TOK_OPERATOR_ASSIGNMENT, // "="
    TOK_OPERATOR_EQUAL, // "=="
    TOK_OPERATOR_NOT_EQUAL, // "!="
    TOK_OPERATOR_LESS_THAN, // "<"
    TOK_OPERATOR_GREATER_THAN, // ">"
    TOK_OPERATOR_LESS_THAN_OR_EQUAL_TO, // "<="
    TOK_OPERATOR_GREATER_THAN_OR_EQUAL_TO, // ">="
    TOK_OPERATOR_LOGICAL_AND, // "&&"
    TOK_OPERATOR_LOGICAL_OR, // "||"
    TOK_OPERATOR_LOGICAL_NOT, // "!"
    TOK_OPERATOR_BITWISE_AND, // "&"
    TOK_OPERATOR_BITWISE_OR, // "|"
    TOK_OPERATOR_BITWISE_XOR, // "^"
    TOK_OPERATOR_BITWISE_NOT, // "~"
    TOK_OPERATOR_LEFT_SHIFT, // "<<"
    TOK_OPERATOR_RIGHT_SHIFT, // ">>"
    TOK_OPERATOR_ADD_ASSIGN, // "+="
    TOK_OPERATOR_SUB_ASSIGN, // "-="
    TOK_OPERATOR_MUL_ASSIGN, // "*="
    TOK_OPERATOR_DIV_ASSIGN, // "/="
    TOK_OPERATOR_MOD_ASSIGN, // "%="
    TOK_OPERATOR_BITWISE_AND_ASSIGN, // "&="
    TOK_OPERATOR_BITWISE_OR_ASSIGN, // "|="
    TOK_OPERATOR_BITWISE_XOR_ASSIGN, // "^="
    TOK_OPERATOR_LEFT_SHIFT_ASSIGN, // "<<="
    TOK_OPERATOR_RIGHT_SHIFT_ASSIGN, // ">>="
    TOK_OPERATOR_DOT, // "."
    TOK_OPERATOR_RANGE, // ".."
    TOK_OPERATOR_DEREFERENCE, // "->"
    TOK_SEPARATOR_COMMA, // ","
    TOK_SEPARATOR_SEMICOLON, // ";"
    TOK_SEPARATOR_COLON, // ":"
    TOK_SEPARATOR_LEFT_PARENTHESIS, // "("
    TOK_SEPARATOR_RIGHT_PARENTHESIS, // ")"
    TOK_SEPARATOR_LEFT_BRACKET, // "["
    TOK_SEPARATOR_RIGHT_BRACKET, // "]"
    TOK_SEPARATOR_LEFT_BRACE, // "{"
    TOK_SEPARATOR_RIGHT_BRACE, // "}"
    TOK_COMMENT_SINGLE_LINE, // Single-line comment
    TOK_COMMENT_MULTIPLE_LINE, // Multi-line comment
    TOK_ERROR, // ERROR TOKEN
    TOK_EOF	// EOF TOKEN
};

// Lexical Analysis
struct list_head *lex(char *source_code);

// Syntax Analysis
ast_node_t *parse(struct list_head *tokens);

// Semantic Analysis
void analyze_semantics(ast_node_t *ast, symbol_table_t *symbol_table);

// Code Generation
void generate_code(ast_node_t *ast, code_generator_t *code_generator);

// Optimization
void optimize_code(code_generator_t *code_generator, optimization_pass_t*
		   optimization_pass);

// Debugging
void debug_code(code_generator_t *code_generator, debug_info_t*
		debug_info);

// Reporting the error and exiting
#define panic(fmt, ...) \
    do { \
        fprintf(stderr, "[PANIC] %s:%d %s(): " fmt, \
                __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)


// Enable it by defining TC_DEBUG in the Makefile
#ifdef TC_DEBUG
    #define tc_debug(level, fmt, ...) \
    do { \
        if (level >= TC_DEBUG) { \
            fprintf(stderr, "[DEBUG] %s:%d %s(): " fmt, \
                    __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        } \
    } while (0)
#else
    #define tc_debug(level, fmt, ...) do {} while (0)
#endif /* TC_DEBUG */

#endif /* TC_H */
