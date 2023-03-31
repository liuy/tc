#include "tc.h"

/* This is a subset of the C language. The grammar is as follows:
 *
 * program = { declaration } ;
 * declaration = var-declaration | fun-declaration ;
 * var-declaration = type-specifier var-declarator-list ";" ;
 * fun-declaration = type-specifier identifier "(" [ param-list ] ")" compound-stmt ;
 * var-declarator-list = var-declarator { "," var-declarator } ;
 * var-declarator = identifier [ "[" num "]" ] ;
 * params-list = param { "," param } ;
 * param = type-specifier param-declarator ;
 * param-declarator = identifier [ "[" num "]" ] ;
 * type-specifier = "int" | "float" | "char" | "void" ;
 * compound-stmt = "{" { declaration | statement } "}" ;
 * statement = expression-stmt | compound-stmt | if-stmt | while-stmt | return-stmt ;
 * expression-stmt = [ expression ] ";" ;
 * if-stmt = "if" "(" expression ")" statement [ "else" statement ] ;
 * while-stmt = "while" "(" expression ")" statement ;
 * return-stmt = "return" [ expression ] ";" ;
 * expression = assign-expression ;
 * assign-expression = equality-expression [ "=" assign-expression ] ;
 * equality-expression = relational-expression { "==" relational-expression } ;
 * relational-expression = additive-expression { ("<" | ">") additive-expression } ;
 * additive-expression = term { ("+" | "-") term } ;
 * term = factor { ("*" | "/") factor } ;
 * factor = identifier | num | "(" expression ")" ;
 * identifier = letter { letter | digit } ;
 * letter = "a" | "b" | ... | "z" | "A" | "B" | ... | "Z" | "_" ;
 * digit = "0" | "1" | ... | "9" ;
 * num = digit { digit } ;
 */

static token_t *current_tok;

static cast_node_t *parse_stmt(void);
static cast_node_t *parse_expr_stmt(void);
static cast_node_t *parse_compound_stmt(void);
static cast_node_t *parse_expr(void);

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
    cast_node_t *node = zalloc(sizeof(cast_node_t));
    node->type = CAST_TYPE_SPECIFIER;
    node->type_specifier.type = current_tok->type;
    eat_current_tok();
    return node;
}

static cast_node_t *parse_num(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    if (current_tok->type != TOK_CONSTANT_INT)
        panic("expected TOK_CONSTANT_INT, but got %s\n", current_tok->lexeme);

    n->type = CAST_NUM;
    n->num.value = atoi(current_tok->lexeme);
    eat_current_tok();
    return n;
}

static cast_node_t *parse_var_declarator(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    if (current_tok->type != TOK_IDENTIFIER)
        panic("identifier expected, but got %s\n", current_tok->lexeme);

    n->type = CAST_VAR_DECLARATOR;
    n->var_declarator.identifier = strndup(current_tok->lexeme, strlen(current_tok->lexeme));

    eat_current_tok();
    if (current_tok->type == TOK_SEPARATOR_LEFT_BRACKET) {
        eat_current_tok(); // eat [
        n->var_declarator.num = parse_num();
        if (current_tok->type != TOK_SEPARATOR_RIGHT_BRACKET)
            panic("']' expected, but got %s\n", current_tok->lexeme);
        eat_current_tok(); // eat ]
    }

    return n;
}

static cast_node_t *parse_var_declarator_list(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_VAR_DECLARATOR_LIST;
    INIT_LIST_HEAD(&n->var_declarator_list.var_declarators);
    list_add_tail(&parse_var_declarator()->list, &n->var_declarator_list.var_declarators);
    while (current_tok->type == TOK_SEPARATOR_COMMA) {
        eat_current_tok(); // eat ','
        list_add_tail(&parse_var_declarator()->list, &n->var_declarator_list.var_declarators);
    }
    return n;
}

static cast_node_t *parse_var_declaration(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_VAR_DECLARATION;
    n->var_declaration.type_specifier = parse_type_specifier();
    n->var_declaration.var_declarator_list = parse_var_declarator_list();
    if (current_tok->type != TOK_SEPARATOR_SEMICOLON)
        panic("';' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ";"
    return n;
}

static cast_node_t *parse_param_declarator(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    if (current_tok->type != TOK_IDENTIFIER)
        panic("identifier expected, but got %s\n", current_tok->lexeme);

    n->type = CAST_PARAM_DECLARATOR;
    n->param_declarator.identifier = strndup(current_tok->lexeme, strlen(current_tok->lexeme));

    eat_current_tok(); // eat identifier
    if (current_tok->type == TOK_SEPARATOR_LEFT_BRACKET) {
        eat_current_tok(); // eat [
        n->param_declarator.num = parse_num();
        if (current_tok->type != TOK_SEPARATOR_RIGHT_BRACKET)
            panic("']' expected, but got %s\n", current_tok->lexeme);
        eat_current_tok(); // eat ]
    }

    return n;
}

static cast_node_t *parse_param(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    if (!is_type_specifier(current_tok))
        panic("type specifier expected, but got %s\n", current_tok->lexeme);

    n->type = CAST_PARAM;
    n->param.type_specifier = parse_type_specifier();
    n->param.param_declarator = parse_param_declarator();
    return n;
}

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

static cast_node_t *parse_factor(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    if (current_tok->type == TOK_CONSTANT_INT) {
        n->factor.num = parse_num();
    } else if (current_tok->type == TOK_SEPARATOR_LEFT_PARENTHESIS) {
        eat_current_tok(); // eat '('
        n->factor.expr = parse_expr();
        if (current_tok->type != TOK_SEPARATOR_RIGHT_PARENTHESIS)
            panic("')' expected, but got %s\n", current_tok->lexeme);
        eat_current_tok(); // eat ')'
    } else if (current_tok->type == TOK_IDENTIFIER) {
        n->factor.identifier = strndup(current_tok->lexeme, strlen(current_tok->lexeme));
        eat_current_tok(); // eat identifier
    } else {
        panic("unexpected token %s\n", current_tok->lexeme);
    }

    return n;
}

static cast_node_t *parse_term(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_TERM;
    INIT_LIST_HEAD(&n->term.terms);
    list_add_tail(&parse_factor()->list, &n->term.terms);
    while (current_tok->type == TOK_OPERATOR_MUL ||
           current_tok->type == TOK_OPERATOR_DIV) {
        eat_current_tok(); // eat "*" or "/"
        list_add_tail(&parse_factor()->list, &n->term.terms);
    }

    return n;
}

static cast_node_t *parse_additive_expression(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_ADDITIVE_EXPR;
    INIT_LIST_HEAD(&n->additive_expr.additive_exprs);
    list_add_tail(&parse_term()->list, &n->additive_expr.additive_exprs);
    while (current_tok->type == TOK_OPERATOR_ADD ||
           current_tok->type == TOK_OPERATOR_SUB) {
        eat_current_tok(); // eat "+" or "-"
        list_add_tail(&parse_term()->list, &n->additive_expr.additive_exprs);
    }

    return n;
}

static cast_node_t *parse_relational_expression(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_RELATIONAL_EXPR;
    INIT_LIST_HEAD(&n->relational_expr.relational_exprs);
    list_add_tail(&parse_additive_expression()->list, &n->relational_expr.relational_exprs);
    while (current_tok->type == TOK_OPERATOR_LESS_THAN ||
           current_tok->type == TOK_OPERATOR_GREATER_THAN) {
        eat_current_tok(); // eat "<" or ">"
        list_add_tail(&parse_additive_expression()->list, &n->relational_expr.relational_exprs);
    }

    return n;
}

static cast_node_t *parse_equality_expression(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_EQUALITY_EXPR;
    INIT_LIST_HEAD(&n->equality_expr.equality_exprs);
    list_add_tail(&parse_relational_expression()->list, &n->equality_expr.equality_exprs);
    while(current_tok->type == TOK_OPERATOR_EQUAL) {
        eat_current_tok(); // eat "=="
        list_add_tail(&parse_relational_expression()->list, &n->equality_expr.equality_exprs);
    }

    return n;
}

static cast_node_t *parse_assign_expression(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_ASSIGN_EXPR;
    n->assign_expr.equality_expr = parse_equality_expression();

    if (current_tok->type == TOK_OPERATOR_ASSIGNMENT) {
        eat_current_tok(); // eat "="
        n->assign_expr.assign_expr = parse_assign_expression();
    }

    return n;
}

static cast_node_t *parse_expr(void)
{
    return parse_assign_expression();
}

static cast_node_t *parse_expr_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_EXPR_STMT;
    n->expr_stmt.expr = parse_expr();
    if (current_tok->type != TOK_SEPARATOR_SEMICOLON)
        panic("';' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ";"
    return n;
}

static cast_node_t *parse_return_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_RETURN_STMT;
    eat_current_tok(); // eat "return"
    n->return_stmt.expr = parse_expr();
    if (current_tok->type != TOK_SEPARATOR_SEMICOLON)
        panic("';' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ";"
    return n;
}

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

static cast_node_t *parse_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    if (current_tok->type == TOK_KEYWORD_IF) {
        n = parse_if_stmt();
    } else if (current_tok->type == TOK_SEPARATOR_LEFT_BRACE) {
        n = parse_compound_stmt();
    } else if (current_tok->type == TOK_KEYWORD_RETURN) {
        n = parse_return_stmt();
    } else if (current_tok->type == TOK_KEYWORD_WHILE) {
        n = parse_while_stmt();
    } else {// TODO: check if it is an expression statement
        n = parse_expr_stmt();
    }
    return n;
}

static cast_node_t *parse_compound_stmt(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_COMPOUND_STMT;
    if (current_tok->type != TOK_SEPARATOR_LEFT_BRACE)
        panic("'{' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat '{'
    INIT_LIST_HEAD(&n->compound_stmt.stmt_list);
    while (current_tok->type != TOK_SEPARATOR_RIGHT_BRACE) {
        cast_node_t *n;

        if (is_type_specifier(current_tok))
            n = parse_var_declaration();
        else
            n = parse_stmt();
        list_add_tail(&n->list, &n->compound_stmt.stmt_list);
    }
    eat_current_tok(); // eat '}'
    return n;
}

static cast_node_t *parse_fun_declaration(void)
{
    cast_node_t *n = zalloc(sizeof(cast_node_t));

    n->type = CAST_FUN_DECLARATION;
    n->fun_declaration.type_specifier = parse_type_specifier();
    if (current_tok->type != TOK_IDENTIFIER)
        panic("identifier expected, but got %s\n", current_tok->lexeme);
    n->fun_declaration.identifier = strndup(current_tok->lexeme, strlen(current_tok->lexeme));
    eat_current_tok(); // eat identifier
    if (current_tok->type != TOK_SEPARATOR_LEFT_PARENTHESIS)
        panic("'(' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat '('
    if (is_type_specifier(current_tok))
        n->fun_declaration.param_list = parse_param_list();
    if (current_tok->type != TOK_SEPARATOR_RIGHT_PARENTHESIS)
        panic("')' expected, but got %s\n", current_tok->lexeme);
    eat_current_tok(); // eat ')'
    n->fun_declaration.compound_stmt = parse_compound_stmt();
    return n;
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
    cast_node_t *p = zalloc(sizeof(cast_node_t));

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
