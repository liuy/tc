#include "tc.h"

/* This is a subset of the C language. The grammar is as follows:
 *
 * program = { declaration } ;
 * declaration = var-declaration | fun-declaration ;
 * var-declaration = type-specifier var-declarator-list ";" ;
 * fun-declaration = type-specifier identifier "(" [ param-list ] ")" compound-stmt ;
 * var-declarator-list = var-declarator { "," var-declarator } ;
 * var-declarator = identifier [ "[" expression "]" | [ "=" expression ] ;
 * params-list = param { "," param } ;
 * param = type-specifier identifier;
 * type-specifier = "int" | "void" ;
 * compound-stmt = "{" { var-declaration | statement } "}" ;
 * statement = assign-stmt | compound-stmt | if-stmt | while-stmt | return-stmt | call-stmt ;
 * assign-stmt = identifier [ "[" expression "]" ] "=" expression ";" ;
 * if-stmt = "if" "(" expression ")" statement [ "else" statement ] ;
 * while-stmt = "while" "(" expression ")" statement ;
 * return-stmt = "return" [ expression ] ";" ;
 * call-stmt = call-expression ";" ;
 * call-expression = identifier "(" [ args-list ] ")" ;
 * args-list = expression { "," expression } ;
 * expression = relational-expression { ("||" | "&&") relational-expression } ;
 * relational-expression = simple-expression [ ("<" | "<= " | ">" | ">=" | "!=" | "==") simple-expression ] ;
 * simple-expression = term { ("+" | "-") term } ;
 * term = factor { ("*" | "/" | "%") factor } ;
 * factor = identifier[ "[" expression "]" ] | num | "(" expression ")" | string  | call-expression ;
 * string = '"' { character } '"' ;
 * identifier = letter { letter | digit } ;
 * letter = "a" | "b" | ... | "z" | "A" | "B" | ... | "Z" | "_" ;
 * digit = "0" | "1" | ... | "9" ;
 * num = digit { digit } ;
 */

static token_t *current_tok;

static cast_node_t *parse_stmt(void);
static cast_node_t *parse_assign_stmt(void);
static cast_node_t *parse_compound_stmt(void);
static cast_node_t *parse_expr(void);
static cast_node_t *parse_call_expression(void);

static inline token_t *next_token(token_t *tok)
{
    return list_next_entry(tok, list);
}

// Eat the current token and move to the next one.
#define eat_current_tok() do { \
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
    return next_tok->type == TOK_SEPARATOR_SEMICOLON ||  //;
           next_tok->type == TOK_SEPARATOR_COMMA ||      //,
           next_tok->type == TOK_OPERATOR_ASSIGN ||      //=
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

// var-declarator = identifier [ "[" num "]" | [ "=" expression ] ;
static cast_node_t *parse_var_declarator(enum token_type type)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    if (current_tok->type != TOK_IDENTIFIER)
        panic("identifier expected, but got %s\n", current_tok->lexeme);

    n->type = CAST_VAR_DECLARATOR;
    n->var_declarator.identifier = strdup(current_tok->lexeme);
    n->var_declarator.type = type;

    eat_current_tok(); // eat identifier
    if (current_tok->type == TOK_OPERATOR_ASSIGN) {
        eat_current_tok(); // eat '='
        n->var_declarator.expr = parse_expr();
    } else if (current_tok->type == TOK_SEPARATOR_LEFT_BRACKET) {
        eat_current_tok(); // eat '['
        if (current_tok->type != TOK_CONSTANT_INT)
            panic("number expected, but got %s\n", current_tok->lexeme);
        n->var_declarator.array_size = atoi(current_tok->lexeme);
        eat_current_tok(); // eat number
        if (current_tok->type != TOK_SEPARATOR_RIGHT_BRACKET)
            panic("']' expected, but got %s\n", current_tok->lexeme);
        eat_current_tok(); // eat ']'
    }

    return n;
}

// var-declarator-list = var-declarator { "," var-declarator } ;
static cast_node_t *parse_var_declarator_list(enum token_type type)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_VAR_DECLARATOR_LIST;
    INIT_LIST_HEAD(&n->var_declarator_list.var_declarators);
    list_add_tail(&parse_var_declarator(type)->list, &n->var_declarator_list.var_declarators);
    while (current_tok->type == TOK_SEPARATOR_COMMA) {
        eat_current_tok(); // eat ','
        list_add_tail(&parse_var_declarator(type)->list, &n->var_declarator_list.var_declarators);
    }
    return n;
}

// var-declaration = type-specifier var-declarator-list ";"
static cast_node_t *parse_var_declaration(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_VAR_DECLARATION;
    n->var_declaration.type = current_tok->type;
    eat_current_tok(); // eat type-specifier
    n->var_declaration.var_declarator_list = parse_var_declarator_list(current_tok->type);
    if (current_tok->type != TOK_SEPARATOR_SEMICOLON)
        panic("';' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ";"
    return n;
}

// param = type_specifier param_declarator
static cast_node_t *parse_param(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    if (!is_type_specifier(current_tok))
        panic("type specifier expected, but got %s\n", current_tok->lexeme);

    n->type = CAST_PARAM;
    n->param.type = current_tok->type;
    eat_current_tok(); // eat type specifier
    if (current_tok->type != TOK_IDENTIFIER)
        panic("identifier expected, but got %s\n", current_tok->lexeme);
    n->param.identifier = strdup(current_tok->lexeme);
    eat_current_tok(); // eat identifier
    return n;
}

// param_list = param {',' param}
static cast_node_t *parse_param_list(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_PARAM_LIST;
    INIT_LIST_HEAD(&n->param_list.params);
    list_add_tail(&parse_param()->list, &n->param_list.params);
    while (current_tok->type == TOK_SEPARATOR_COMMA) {
        eat_current_tok(); // eat ','
        list_add_tail(&parse_param()->list, &n->param_list.params);
    }
    return n;
}

// factor = num | '(' expr ')' | identifier | call_expr | string
static cast_node_t *parse_factor(void)
{
    cast_node_t *n = NULL;

    if (current_tok->type == TOK_IDENTIFIER) {
        token_t *ntok = next_token(current_tok);
        if (ntok->type == TOK_SEPARATOR_LEFT_PARENTHESIS) {
            return parse_call_expression();
        } else {
            n = zalloc(sizeof(cast_node_t));
            n->type = CAST_IDENTIFIER;
            n->expr.identifier = strdup(current_tok->lexeme);
            if (ntok->type == TOK_SEPARATOR_LEFT_BRACKET) {
                eat_current_tok(); // eat identifier
                eat_current_tok(); // eat '['
                n->expr.array_expr = parse_expr();
                if (current_tok->type != TOK_SEPARATOR_RIGHT_BRACKET)
                    panic("']' expected, but got %s\n", current_tok->lexeme);
                eat_current_tok(); // eat ']'
            } else {
                eat_current_tok(); // eat identifier
            }
        }
    } else if (current_tok->type == TOK_CONSTANT_INT) {
        n = zalloc(sizeof(cast_node_t));
        n->type = CAST_NUMBER;
        n->expr.num = atoi(current_tok->lexeme);
        eat_current_tok(); // eat number
    } else if (current_tok->type == TOK_SEPARATOR_LEFT_PARENTHESIS) {
        eat_current_tok(); // eat "("
        n = parse_expr();
        if (current_tok->type != TOK_SEPARATOR_RIGHT_PARENTHESIS)
            panic("')' expected, but got %s\n", current_tok->lexeme);
        eat_current_tok(); // eat ")"
    } else if (current_tok->type == TOK_CONSTANT_STRING) {
        n = zalloc(sizeof(cast_node_t));
        n->type = CAST_STRING;
        n->expr.string = strdup(current_tok->lexeme);
        eat_current_tok(); // eat string
    } else {
        panic("Expected string, identifier, number, or '(', but got %s\n", current_tok->lexeme);
    }

    return n;
}

// term = factor { ("*" | "/") factor }
static cast_node_t *parse_term(void)
{
    cast_node_t *n = parse_factor();

    while (current_tok->type == TOK_OPERATOR_MUL ||
           current_tok->type == TOK_OPERATOR_DIV ||
           current_tok->type == TOK_OPERATOR_MOD) {
        cast_node_t *op_node = zalloc(sizeof(cast_node_t));
        op_node->type = CAST_TERM;
        op_node->expr.op.type = current_tok->type;
        op_node->expr.op.left = n;
        eat_current_tok(); // eat "*" or "/" or "%"
        op_node->expr.op.right = parse_factor();
        n = op_node;
    }

    return n;
}

// simple-expression = term { ("+" | "-") term } ;
static cast_node_t *parse_simple_expr(void)
{
    cast_node_t *n = parse_term();

    while (current_tok->type == TOK_OPERATOR_ADD ||
           current_tok->type == TOK_OPERATOR_SUB) {
        cast_node_t *op_node = zalloc(sizeof(cast_node_t));
        op_node->type = CAST_SIMPLE_EXPR;
        op_node->expr.op.type = current_tok->type;
        op_node->expr.op.left = n;
        eat_current_tok(); // eat "+" or "-"
        op_node->expr.op.right = parse_term();
        n = op_node;
    }

    return n;
}

// relational-expression = simple-expression [ ("<" | "<= " | ">" | ">=" | "!=" | "==") simple-expression ] ;
static cast_node_t *parse_relational_expr(void)
{
    cast_node_t *n = parse_simple_expr();

    while (current_tok->type == TOK_OPERATOR_LESS_THAN ||
           current_tok->type == TOK_OPERATOR_LESS_THAN_OR_EQUAL_TO ||
           current_tok->type == TOK_OPERATOR_GREATER_THAN ||
           current_tok->type == TOK_OPERATOR_GREATER_THAN_OR_EQUAL_TO ||
           current_tok->type == TOK_OPERATOR_NOT_EQUAL ||
           current_tok->type == TOK_OPERATOR_EQUAL) {
        cast_node_t *op_node = zalloc(sizeof(cast_node_t));
        op_node->type = CAST_RELATIONAL_EXPR;
        op_node->expr.op.type = current_tok->type;
        op_node->expr.op.left = n;
        eat_current_tok(); // eat "<" or "<=" or ">" or ">=" or "!=" or "=="
        op_node->expr.op.right = parse_simple_expr();
        n = op_node;
    }

    return n;
}

// call-expression = identifier "(" [ args-list ] ")" ;
static cast_node_t *parse_call_expression(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_CALL_EXPR;
    n->call_expr.identifier = strdup(current_tok->lexeme);

    eat_current_tok(); // eat identifier
    eat_current_tok(); // eat '('

    // parse args-list
    INIT_LIST_HEAD(&n->call_expr.args_list);
    if (current_tok->type != TOK_SEPARATOR_RIGHT_PARENTHESIS) {
        list_add_tail(&parse_expr()->list, &n->call_expr.args_list);
        while (current_tok->type == TOK_SEPARATOR_COMMA) {
            eat_current_tok(); // eat ','
            list_add_tail(&parse_expr()->list, &n->call_expr.args_list);
        }
    }

    if (current_tok->type != TOK_SEPARATOR_RIGHT_PARENTHESIS)
        panic("')' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ')'

    return n;
}

// expression = relational-expression { ("||" | "&&") relational-expression } ;
static cast_node_t *parse_expr(void)
{
    cast_node_t *n;

    n = parse_relational_expr();

    while (current_tok->type == TOK_OPERATOR_LOGICAL_OR ||
           current_tok->type == TOK_OPERATOR_LOGICAL_AND) {
        cast_node_t *op_node = zalloc(sizeof(cast_node_t));
        op_node->type = CAST_LOGICAL_EXPR;
        op_node->expr.op.type = current_tok->type;
        op_node->expr.op.left = n;
        eat_current_tok(); // eat "||" or "&&"
        op_node->expr.op.right = parse_relational_expr();
        n = op_node;
    }

    return n;
}

// assgin_stmt = identifier [ "[" expression "]" ] "=" expression ;
static cast_node_t *parse_assign_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_ASSIGN_STMT;
    n->assign_stmt.identifier = strdup(current_tok->lexeme);
    eat_current_tok(); // eat identifier

    if (current_tok->type == TOK_SEPARATOR_LEFT_BRACKET) {
        eat_current_tok(); // eat '['
        n->assign_stmt.array_expr = parse_expr();
        if (current_tok->type != TOK_SEPARATOR_RIGHT_BRACKET)
            panic("']' expected, but got %s\n", current_tok->lexeme);
        eat_current_tok(); // eat ']'
    }

    if (current_tok->type != TOK_OPERATOR_ASSIGN)
        panic("'=' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat "="
    n->assign_stmt.expr = parse_expr();

    if (current_tok->type != TOK_SEPARATOR_SEMICOLON)
        panic("';' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ";"
    return n;
}

// return [expr];
static cast_node_t *parse_return_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_RETURN_STMT;
    eat_current_tok(); // eat "return"
    // Allow empty return statement
    if (current_tok->type != TOK_SEPARATOR_SEMICOLON)
        n->return_stmt.expr = parse_expr();
    if (current_tok->type != TOK_SEPARATOR_SEMICOLON)
        panic("';' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ";"
    return n;
}

// while (expr) stmt
static cast_node_t *parse_while_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_WHILE_STMT;
    eat_current_tok(); // eat "while"
    if (current_tok->type != TOK_SEPARATOR_LEFT_PARENTHESIS)
        panic("'(' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat '('
    n->while_stmt.expr = parse_expr();
    if (current_tok->type != TOK_SEPARATOR_RIGHT_PARENTHESIS)
        panic("')' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ')'
    n->while_stmt.stmt = parse_stmt();
    return n;
}

// if (expr) stmt [else stmt]
static cast_node_t *parse_if_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_IF_STMT;
    eat_current_tok(); // eat "if"
    if (current_tok->type != TOK_SEPARATOR_LEFT_PARENTHESIS)
        panic("'(' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat '('
    n->if_stmt.expr = parse_expr();
    if (current_tok->type != TOK_SEPARATOR_RIGHT_PARENTHESIS)
        panic("')' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ')'
    n->if_stmt.if_stmt = parse_stmt();
    if (current_tok->type == TOK_KEYWORD_ELSE) {
        eat_current_tok(); // eat "else"
        n->if_stmt.else_stmt = parse_stmt();
    }
    return n;
}

// call-stmt = call-expr ";"
static cast_node_t *parse_call_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));
    n->type = CAST_CALL_STMT;
    n->call_stmt.expr = parse_call_expression();
    eat_current_tok(); // eat ';'
    return n;
}

// stmt = if_stmt | compound_stmt | return_stmt | while_stmt | assign_stmt | call_stmt
static cast_node_t *parse_stmt(void)
{
    if (current_tok->type == TOK_KEYWORD_IF)
        return parse_if_stmt();
    else if (current_tok->type == TOK_KEYWORD_WHILE)
        return parse_while_stmt();
    else if (current_tok->type == TOK_KEYWORD_RETURN)
        return parse_return_stmt();
    else if (current_tok->type == TOK_IDENTIFIER) {
        token_t *next_tok = next_token(current_tok);
        if (next_tok->type == TOK_SEPARATOR_LEFT_PARENTHESIS)
            return parse_call_stmt();
        else
            return parse_assign_stmt();
    } else if (current_tok->type == TOK_SEPARATOR_LEFT_BRACE)
        return parse_compound_stmt();
    else
        panic("unexpected token %s\n", current_tok->lexeme);
}

// compound_stmt = "{" {var_declaration | stmt} "}"
static cast_node_t *parse_compound_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_COMPOUND_STMT;
    if (current_tok->type != TOK_SEPARATOR_LEFT_BRACE)
        panic("'{' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat '{'
    INIT_LIST_HEAD(&n->compound_stmt.stmts);
    while (current_tok->type != TOK_SEPARATOR_RIGHT_BRACE) {
        cast_node_t *node;

        while (current_tok->type == TOK_SEPARATOR_SEMICOLON)
            eat_current_tok(); // eat extra ";" if it exists

        if (current_tok->type == TOK_SEPARATOR_RIGHT_BRACE)
            break;// "{ ; }" is valid

        if (current_tok->type == TOK_EOF)
            panic("'}' expected, but got EOF\n");

        if (is_type_specifier(current_tok))
            node = parse_var_declaration();
        else
            node = parse_stmt();
        list_add_tail(&node->list, &n->compound_stmt.stmts);
    }
    eat_current_tok(); // eat '}'
    return n;
}

// fun_declaration = type_specifier identifier "(" [param_list] ")" compound_stmt
static cast_node_t *parse_fun_declaration(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_FUN_DECLARATION;
    n->fun_declaration.type = current_tok->type;
    eat_current_tok(); // eat type_specifier
    if (current_tok->type != TOK_IDENTIFIER)
        panic("identifier expected, but got %s\n", current_tok->lexeme);
    n->fun_declaration.identifier = strdup(current_tok->lexeme);
    eat_current_tok(); // eat identifier
    if (current_tok->type != TOK_SEPARATOR_LEFT_PARENTHESIS)
        panic("'(' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat '('
    if (is_type_specifier(current_tok))
        n->fun_declaration.param_list = parse_param_list();
    if (current_tok->type != TOK_SEPARATOR_RIGHT_PARENTHESIS)
        panic("')' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ')'
    if (current_tok->type == TOK_SEPARATOR_LEFT_BRACE)
        n->fun_declaration.compound_stmt = parse_compound_stmt();
    else if (current_tok->type == TOK_SEPARATOR_SEMICOLON)
        eat_current_tok(); // eat ';'
    else
        panic("';' or '{' expected, but got %s\n", current_tok->lexeme);
    return n;
}

// declaration = var_declaration | fun_declaration
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

// program = {declaration}
static cast_node_t *parse_program(void)
{
    cast_node_t *p = zalloc(sizeof(cast_node_t));

    p->type = CAST_PROGRAM;
    INIT_LIST_HEAD(&p->program.declarations);
    while (current_tok->type != TOK_EOF) {
        while (current_tok->type == TOK_SEPARATOR_SEMICOLON)
            eat_current_tok(); // eat extra ";" if it exists
        if (current_tok->type == TOK_EOF)
            goto out; // end of file
        cast_node_t *d = parse_declaration();
        list_add_tail(&d->list, &p->program.declarations);
    }
out:
    return p;
}

cast_node_t *parse(struct list_head *token_stream)
{
    // init current_tok
    current_tok = list_first_entry(token_stream, token_t, list);
    cast_node_t *p = parse_program();
    return p;
}
