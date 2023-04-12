#ifndef TC_H
#define TC_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

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
    TOK_OPERATOR_ASSIGN, // "="
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
    TOK_NULL, // NULL TOKEN
    TOK_EOF	// EOF TOKEN
};

typedef struct token {
    struct list_node list;
    char *lexeme;
    enum token_type type;
} token_t;

typedef struct symbol {
    struct hlist_node list;
    char *name;
    enum token_type type;
    // variable specific
    int index; // for stack index, 0 means global
    // functioin specific
    int arg_count; // used by generator
    int var_count; // used by generator
} symbol_t;

/*
 * Scopes are implemented as linked lists of symbol tables.
 * There is one file scope and nested scopes for functions.
 */
typedef struct symbol_table {
#define TABLE_SIZE 1024
    struct hlist_head table[TABLE_SIZE];
    struct symbol_table *parent;
    char *name;
} symbol_table_t;

static inline int symbol_table_hash(char *str)
{
    return fnv1_hash(str) % TABLE_SIZE;
}
enum cast_node_type {
    CAST_PROGRAM,
    CAST_DECLARATION,
    CAST_VAR_DECLARATION,
    CAST_TYPE_SPECIFIER,
    CAST_VAR_DECLARATOR_LIST,
    CAST_VAR_DECLARATOR,
    CAST_FUN_DECLARATION,
    CAST_PARAM_LIST,
    CAST_PARAM,
    CAST_PARAM_DECLARATOR,
    CAST_COMPOUND_STMT,
    CAST_STMT,
    CAST_ASSIGN_STMT,
    CAST_IF_STMT,
    CAST_WHILE_STMT,
    CAST_RETURN_STMT,
    CAST_CALL_STMT,
    CAST_EXPR,
    CAST_CALL_EXPR,
    CAST_RELATIONAL_EXPR,
    CAST_SIMPLE_EXPR,
    CAST_TERM,
    CAST_FACTOR,
    CAST_IDENTIFIER,
    CAST_NUMBER,
    CAST_STRING
};

// C Abstract Syntax Tree (CAST) node
typedef struct cast_node {
    struct list_node list;
    enum cast_node_type type;
    int line_number;
    union {
        struct {
            struct list_head declarations;
            symbol_table_t *symbol_table;
        } program;
//        struct {
//            struct cast_node *specifier;
//            struct cast_node *declarator;
//        } declaration;
        struct {
            enum token_type type;
            struct cast_node *var_declarator_list;
        } var_declaration;
        struct {
            struct list_head var_declarators;
        } var_declarator_list;
        struct {
            char *identifier;
            struct cast_node *expr;
        } var_declarator;
        struct {
            enum token_type type;
            char *identifier;
            struct cast_node *param_list;
            struct cast_node *compound_stmt;
            symbol_table_t *symbol_table;
        } fun_declaration;
        struct {
            struct list_head params;
        } param_list;
        struct {
            enum token_type type;
            char *identifier;
        } param;
        struct {
            struct list_head stmts;
            symbol_table_t *symbol_table;
        } compound_stmt;
        struct {
            char *identifier;
            struct cast_node *expr;
        } assign_stmt;
        struct {
            struct cast_node *expr;
            struct cast_node *if_stmt;
            struct cast_node *else_stmt;
        } if_stmt;
        struct {
            struct cast_node *expr;
            struct cast_node *stmt;
        } while_stmt;
        struct {
            struct cast_node *expr;
        } return_stmt;
        struct {
            struct cast_node *expr;
        } call_stmt;
        struct {
            char *identifier;
            struct list_head args_list;
        } call_expr;
        union {
            struct {
                enum token_type type;
                struct cast_node *left;
                struct cast_node *right;
            } op;
            int num;
            char *identifier;
            char *string;
            struct cast_node *expr;
        } expr;
    };
} cast_node_t;

// Lexical Analysis and helpers in lex.c
struct list_head *lex(char *source_code);
const char *token_type_to_str(enum token_type type);

static inline const char *token_to_str(token_t *token)
{
    return token_type_to_str(token->type);
}

static inline int is_keyword(token_t *token)
{
    return token->type >= TOK_KEYWORD_AUTO &&
           token->type <= TOK_KEYWORD_WHILE;
}

static inline int is_operator(token_t *token)
{
    return token->type >= TOK_OPERATOR_ADD &&
           token->type <= TOK_OPERATOR_RIGHT_SHIFT_ASSIGN;
}

static inline int is_constant(token_t *token)
{
    return token->type >= TOK_CONSTANT_INT &&
           token->type <= TOK_CONSTANT_STRING;
}
static inline int is_separator(token_t *token)
{
    return token->type >= TOK_SEPARATOR_COMMA &&
           token->type <= TOK_SEPARATOR_RIGHT_BRACE;
}

static inline int token_is(token_t *token, const char *str)
{
    return strcmp(token->lexeme, str) == 0;
}

// Syntax Analysis
cast_node_t *parse(struct list_head *tokens);

// Semantic Analysis
void analyze_semantics(cast_node_t *ast);
symbol_t *symbol_table_lookup(symbol_table_t *t, char *name, int upward);

// Code Generation
void generate_code(cast_node_t *ast);

// Optimization
void optimize_code();

// Debugging
void debug_code();

// Reporting the error and exiting
#define panic(fmt, ...) \
    do { \
        fprintf(stderr, "[PANIC] %s:%d %s(): " fmt, \
                __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)

#define not_implemented() \
    panic("Not implemented function %s\n", __func__)


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

#define zalloc(size) ({ \
    void *ptr = malloc(size); \
    if (!ptr) { \
        panic("Out of memory\n"); \
    } \
    memset(ptr, 0, size); \
    ptr; \
})

#endif /* TC_H */
