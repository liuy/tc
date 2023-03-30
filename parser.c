#include "tc.h"

/* This is a subset of the C language. The grammar is as follows:
 *
 * program = { declaration } ;
 * declaration = var-declaration | fun-declaration ;
 * var-declaration = type-specifier var-declarator-list ";" ;
 * fun-declaration = type-specifier identifier "(" [ params ] ")" compound-stmt ;
 * var-declarator-list = var-declarator { "," var-declarator } ;
 * var-declarator = identifier [ "[" num "]" ] ;
 * params = param { "," param } ;
 * param = type-specifier declarator ;
 * declarator = identifier [ "[" num "]" ] ;
 * type-specifier = "int" | "float" | "char" | "void" ;
 * compound-stmt = "{" { declaration | statement } "}" ;
 * statement = exprGession-stmt | compound-stmt | if-stmt | while-stmt | return-stmt ;
 * expression-stmt = [ expression ] ";" ;
 * if-stmt = "if" "(" expression ")" statement [ "else" statement ] ;
 * while-stmt = "while" "(" expression ")" statement ;
 * return-stmt = "return" [ expression ] ";" ;
 * expression = assign-expression ;
 * assign-expression = equality-expression [ "=" assign-expression ] ;
 * equality-expression = relational-expression { "==" relational-expression } ;
 * relational-expression = additive-expression { "<" additive-expression } ;
 * additive-expression = term { ("+" | "-") term } ;
 * term = factor { ("*" | "/") factor } ;
 * factor = identifier | num | "(" expression ")" ;
 * identifier = letter { letter | digit } ;
 * letter = "a" | "b" | ... | "z" | "A" | "B" | ... | "Z" | "_" ;
 * digit = "0" | "1" | ... | "9" ;
 * num = digit { digit } ;
 */

static token_t *current_tok;

static inline token_t *next_token(token_t *tok)
{
    return list_next_entry(tok, list);
}

// Eat the current token and move to the next one.
#define current_tok_eat() do { \
    tc_debug(0, "[%s] is parsed\n", current_tok->lexeme); \
    current_tok = next_token(current_tok); \
} while(0)

static inline int is_type_specifier(token_t *tok)
{
    return tok->type == TOK_KEYWORD_INT || tok->type == TOK_KEYWORD_FLOAT ||
           tok->type == TOK_KEYWORD_CHAR || tok->type == TOK_KEYWORD_VOID;
}

static inline int possible_var_declarator(token_t *tok)
{
    token_t *next_tok;
    if (tok->type != TOK_IDENTIFIER)
        return 0;

    next_tok = next_token(tok);
    return next_tok->type == TOK_SEPARATOR_SEMICOLON || //;
           next_tok->type == TOK_SEPARATOR_COMMA || //,
           next_tok->type == TOK_SEPARATOR_LEFT_BRACKET; //[
}

static inline int possible_fun_declarator(token_t *tok)
{
    token_t *next_tok;
    if (tok->type != TOK_IDENTIFIER)
        return 0;

    next_tok = next_token(tok);
    return next_tok->type == TOK_SEPARATOR_LEFT_PARENTHESIS; //(
}


static cast_node_t *parse_type_specifier(void)
{
    cast_node_t *node = malloc(sizeof(cast_node_t));
    node->type = CAST_TYPE_SPECIFIER;
    node->type_specifier.type = current_tok->type;
    current_tok_eat();
    return node;
}

static cast_node_t *parse_num(void)
{
    cast_node_t *n = malloc(sizeof(cast_node_t));

    if (current_tok->type != TOK_CONSTANT_INT)
        panic("expected TOK_CONSTANT_INT, but got %s\n", current_tok->lexeme);

    n->type = CAST_NUM;
    n->num.value = atoi(current_tok->lexeme);
    current_tok_eat();
    return n;
}

static cast_node_t *parse_var_declarator(void)
{
    cast_node_t *n = malloc(sizeof(cast_node_t));

    if (current_tok->type != TOK_IDENTIFIER)
        panic("identifier expected, but got %s\n", current_tok->lexeme);

    n->type = CAST_VAR_DECLARATOR;
    n->var_declarator.identifier = strndup(current_tok->lexeme, strlen(current_tok->lexeme));
    current_tok_eat();
    if (current_tok->type == TOK_SEPARATOR_LEFT_BRACKET) {
        current_tok_eat(); // eat [
        n->var_declarator.num = parse_num();
        if (current_tok->type != TOK_SEPARATOR_RIGHT_BRACKET)
            panic("']' expected, but got %s\n", current_tok->lexeme);
        current_tok_eat(); // eat ]
    }

    return n;
}

static cast_node_t *parse_var_declarator_list(void)
{
    cast_node_t *n = malloc(sizeof(cast_node_t));

    n->type = CAST_VAR_DECLARATOR_LIST;
    INIT_LIST_HEAD(&n->var_declarator_list.var_declarators);
    list_add_tail(&parse_var_declarator()->list, &n->var_declarator_list.var_declarators);
    while (current_tok->type == TOK_SEPARATOR_COMMA) {
        current_tok_eat(); // eat ','
        list_add_tail(&parse_var_declarator()->list, &n->var_declarator_list.var_declarators);
    }
    return n;
}

static cast_node_t *parse_var_declaration(void)
{
    cast_node_t *n = malloc(sizeof(cast_node_t));

    n->type = CAST_VAR_DECLARATION;
    n->var_declaration.type_specifier = parse_type_specifier();
    n->var_declaration.var_declarator_list = parse_var_declarator_list();
    if (current_tok->type != TOK_SEPARATOR_SEMICOLON)
        panic("';' expected, but got %s\n", current_tok->lexeme);
    current_tok_eat(); // eat ";"
    return n;
}

static cast_node_t *parse_fun_declaration(void)
{
    not_implemented();
}

static cast_node_t *parse_declaration(void)
{
    token_t *next_tok = next_token(current_tok);

    if (!is_type_specifier(current_tok))
        panic("Expected type specifier, but got %s\n", current_tok->lexeme);

    if (possible_var_declarator(next_tok)) {
        return parse_var_declaration();
    } else if (possible_fun_declarator(next_tok)) {
        return parse_fun_declaration();
    } else {
        panic("Expected var or fun declarator\n");
    }
}


static cast_node_t *parse_program(void)
{
    cast_node_t *p = malloc(sizeof(cast_node_t));

    p->type = CAST_PROGRAM;
    INIT_LIST_HEAD(&p->program.declarations);
    while (current_tok->type != TOK_EOF) {
        cast_node_t *d = parse_declaration();
        list_add_tail(&d->list, &p->program.declarations);
    }
    return p;
}

cast_node_t *parse(struct list_head *token_stream)
{
    // init current_tok
    current_tok = list_first_entry(token_stream, token_t, list);
    cast_node_t *p = parse_program();
    return p;
}
