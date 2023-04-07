#include <stdlib.h>
#include <check.h>
#include "../tc.h"

static void token_free(token_t *tok)
{
    free(tok->lexeme);
    free(tok);
}

static int check_cmd(const char *command, const char *expected)
{
    char buffer[1024];
    FILE *fp;

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command %s\n", command );
        return -1;
    }

    /* Read the output a line at a time - output it. */
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, expected) != NULL) {
            /* Expected output is found, close the pipe and return 1 */
            pclose(fp);
            return 1;
        }
    }

    /* Expected output not found, return 0 */
    pclose(fp);
    return 0;
}

/**********************************************************************
 * Lexer tests
 **********************************************************************/
START_TEST(test_lex_numbers)
{
    char *input = "123 123456789l 123.456";
    //TODO: add 0x123 0b1010 0.123e-10 0.123e+10 0.123e10

    struct list_head *tokens = lex(input);
    //ck_assert_ptr_nonnull(tokens); supported since 0.11.0
    ck_assert_int_eq(list_size(tokens), 4);

    token_t *tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_CONSTANT_INT);
    ck_assert_str_eq(tok->lexeme, "123");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_CONSTANT_LONG);
    ck_assert_str_eq(tok->lexeme, "123456789l");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_CONSTANT_FLOAT);
    ck_assert_str_eq(tok->lexeme, "123.456");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_EOF);
    token_free(tok);
}
END_TEST

START_TEST(test_lex_keywords)
{
    char *input = "auto break case char const continue default do double else \
                   enum extern float for goto if int long register return short \
                   signed sizeof static struct switch typedef union unsigned \
                   void volatile while";
    struct list_head *tokens = lex(input);
    //ck_assert_ptr_nonnull(tokens);
    ck_assert_int_eq(list_size(tokens), 33);

    token_t *tok;
    list_for_each_entry(tok, tokens, list) {
        if (tok->type == TOK_KEYWORD_AUTO) {
            ck_assert_str_eq(tok->lexeme, "auto");
        } else if (tok->type == TOK_KEYWORD_BREAK) {
            ck_assert_str_eq(tok->lexeme, "break");
        } else if (tok->type == TOK_KEYWORD_CASE) {
            ck_assert_str_eq(tok->lexeme, "case");
        } else if (tok->type == TOK_KEYWORD_CHAR) {
            ck_assert_str_eq(tok->lexeme, "char");
        } else if (tok->type == TOK_KEYWORD_CONST) {
            ck_assert_str_eq(tok->lexeme, "const");
        } else if (tok->type == TOK_KEYWORD_CONTINUE) {
            ck_assert_str_eq(tok->lexeme, "continue");
        } else if (tok->type == TOK_KEYWORD_DEFAULT) {
            ck_assert_str_eq(tok->lexeme, "default");
        } else if (tok->type == TOK_KEYWORD_DO) {
            ck_assert_str_eq(tok->lexeme, "do");
        } else if (tok->type == TOK_KEYWORD_DOUBLE) {
            ck_assert_str_eq(tok->lexeme, "double");
        } else if (tok->type == TOK_KEYWORD_ELSE) {
            ck_assert_str_eq(tok->lexeme, "else");
        } else if (tok->type == TOK_KEYWORD_ENUM) {
            ck_assert_str_eq(tok->lexeme, "enum");
        } else if (tok->type == TOK_KEYWORD_EXTERN) {
            ck_assert_str_eq(tok->lexeme, "extern");
        } else if (tok->type == TOK_KEYWORD_FLOAT) {
            ck_assert_str_eq(tok->lexeme, "float");
        } else if (tok->type == TOK_KEYWORD_FOR) {
            ck_assert_str_eq(tok->lexeme, "for");
        } else if (tok->type == TOK_KEYWORD_GOTO) {
            ck_assert_str_eq(tok->lexeme, "goto");
        } else if (tok->type == TOK_KEYWORD_IF) {
            ck_assert_str_eq(tok->lexeme, "if");
        } else if (tok->type == TOK_KEYWORD_INT) {
            ck_assert_str_eq(tok->lexeme, "int");
        } else if (tok->type == TOK_KEYWORD_LONG) {
            ck_assert_str_eq(tok->lexeme, "long");
        } else if (tok->type == TOK_KEYWORD_REGISTER) {
            ck_assert_str_eq(tok->lexeme, "register");
        } else if (tok->type == TOK_KEYWORD_RETURN) {
            ck_assert_str_eq(tok->lexeme, "return");
        } else if (tok->type == TOK_KEYWORD_SHORT) {
            ck_assert_str_eq(tok->lexeme, "short");
        } else if (tok->type == TOK_KEYWORD_SIGNED) {
            ck_assert_str_eq(tok->lexeme, "signed");
        } else if (tok->type == TOK_KEYWORD_SIZEOF) {
            ck_assert_str_eq(tok->lexeme, "sizeof");
        } else if (tok->type == TOK_KEYWORD_STATIC) {
            ck_assert_str_eq(tok->lexeme, "static");
        } else if (tok->type == TOK_KEYWORD_STRUCT) {
            ck_assert_str_eq(tok->lexeme, "struct");
        } else if (tok->type == TOK_KEYWORD_SWITCH) {
            ck_assert_str_eq(tok->lexeme, "switch");
        } else if (tok->type == TOK_KEYWORD_TYPEDEF) {
            ck_assert_str_eq(tok->lexeme, "typedef");
        } else if (tok->type == TOK_KEYWORD_UNION) {
            ck_assert_str_eq(tok->lexeme, "union");
        } else if (tok->type == TOK_KEYWORD_UNSIGNED) {
            ck_assert_str_eq(tok->lexeme, "unsigned");
        } else if (tok->type == TOK_KEYWORD_VOID) {
            ck_assert_str_eq(tok->lexeme, "void");
        } else if (tok->type == TOK_KEYWORD_VOLATILE) {
            ck_assert_str_eq(tok->lexeme, "volatile");
        } else if (tok->type == TOK_KEYWORD_WHILE) {
            ck_assert_str_eq(tok->lexeme, "while");
        }
	list_del(&tok->list);
	token_free(tok);
    }
}
END_TEST

START_TEST(test_lex_operators)
{
    char *input = "+ - * / % ++ -- == != > < >= <= && || ! & | ^ ~ << >> += -= \
                   *= /= %= &= |= ^= <<= >>=";
    struct list_head *tokens = lex(input);
    //ck_assert_ptr_nonnull(tokens);
    ck_assert_int_eq(list_size(tokens), 33);

    token_t *tok;
    list_for_each_entry(tok, tokens, list) {
        if (tok->type == TOK_OPERATOR_ADD) {
            ck_assert_str_eq(tok->lexeme, "+");
        } else if (tok->type == TOK_OPERATOR_SUB) {
            ck_assert_str_eq(tok->lexeme, "-");
        } else if (tok->type == TOK_OPERATOR_MUL) {
            ck_assert_str_eq(tok->lexeme, "*");
        } else if (tok->type == TOK_OPERATOR_DIV) {
            ck_assert_str_eq(tok->lexeme, "/");
        } else if (tok->type == TOK_OPERATOR_MOD) {
            ck_assert_str_eq(tok->lexeme, "%");
        } else if (tok->type == TOK_OPERATOR_INC) {
            ck_assert_str_eq(tok->lexeme, "++");
        } else if (tok->type == TOK_OPERATOR_DEC) {
            ck_assert_str_eq(tok->lexeme, "--");
        } else if (tok->type == TOK_OPERATOR_EQUAL) {
            ck_assert_str_eq(tok->lexeme, "==");
        } else if (tok->type == TOK_OPERATOR_NOT_EQUAL) {
            ck_assert_str_eq(tok->lexeme, "!=");
        } else if (tok->type == TOK_OPERATOR_GREATER_THAN) {
            ck_assert_str_eq(tok->lexeme, ">");
        } else if (tok->type == TOK_OPERATOR_LESS_THAN) {
            ck_assert_str_eq(tok->lexeme, "<");
        } else if (tok->type == TOK_OPERATOR_GREATER_THAN_OR_EQUAL_TO) {
            ck_assert_str_eq(tok->lexeme, ">=");
        } else if (tok->type == TOK_OPERATOR_LESS_THAN_OR_EQUAL_TO) {
            ck_assert_str_eq(tok->lexeme, "<=");
        } else if (tok->type == TOK_OPERATOR_LOGICAL_AND) {
            ck_assert_str_eq(tok->lexeme, "&&");
        } else if (tok->type == TOK_OPERATOR_LOGICAL_OR) {
            ck_assert_str_eq(tok->lexeme, "||");
        } else if (tok->type == TOK_OPERATOR_BITWISE_AND) {
            ck_assert_str_eq(tok->lexeme, "&");
        } else if (tok->type == TOK_OPERATOR_BITWISE_OR) {
            ck_assert_str_eq(tok->lexeme, "|");
        } else if (tok->type == TOK_OPERATOR_BITWISE_XOR) {
            ck_assert_str_eq(tok->lexeme, "^");
        } else if (tok->type == TOK_OPERATOR_BITWISE_NOT) {
            ck_assert_str_eq(tok->lexeme, "~");
        } else if (tok->type == TOK_OPERATOR_LEFT_SHIFT) {
            ck_assert_str_eq(tok->lexeme, "<<");
        } else if (tok->type == TOK_OPERATOR_RIGHT_SHIFT) {
            ck_assert_str_eq(tok->lexeme, ">>");
        } else if (tok->type == TOK_OPERATOR_ADD_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "+=");
        } else if (tok->type == TOK_OPERATOR_SUB_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "-=");
        } else if (tok->type == TOK_OPERATOR_MUL_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "*=");
        } else if (tok->type == TOK_OPERATOR_DIV_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "/=");
        } else if (tok->type == TOK_OPERATOR_MOD_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "%=");
        } else if (tok->type == TOK_OPERATOR_BITWISE_AND_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "&=");
        } else if (tok->type == TOK_OPERATOR_BITWISE_OR_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "|=");
        } else if (tok->type == TOK_OPERATOR_BITWISE_XOR_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "^=");
        } else if (tok->type == TOK_OPERATOR_LEFT_SHIFT_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, "<<=");
        } else if (tok->type == TOK_OPERATOR_RIGHT_SHIFT_ASSIGN) {
            ck_assert_str_eq(tok->lexeme, ">>=");
        }
	list_del(&tok->list);
	token_free(tok);
    }
}
END_TEST

START_TEST(test_lex_strings_and_chars)
{
    char *input = "\"Hello, world!\n\" \"ChatGPT\" 'c' '\n'";
    struct list_head *tokens = lex(input);
    ck_assert_int_eq(list_size(tokens), 5);

    token_t *tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_CONSTANT_STRING);
    ck_assert_str_eq(tok->lexeme, "\"Hello, world!\n\"");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_CONSTANT_STRING);
    ck_assert_str_eq(tok->lexeme, "\"ChatGPT\"");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_CONSTANT_CHAR);
    ck_assert_str_eq(tok->lexeme, "'c'");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_CONSTANT_CHAR);
    ck_assert_str_eq(tok->lexeme, "'\n'");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_EOF);
    token_free(tok);
}
END_TEST

START_TEST(test_lex_whitespaces_and_comments)
{
    char *input = "/* This is a comment */ int main()\n {\n \treturn 0;\n } \
        // Another comment";
    struct list_head *tokens = lex(input);
    token_t *tok;

    ck_assert_int_eq(list_size(tokens), 10);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_KEYWORD_INT);
    ck_assert_str_eq(tok->lexeme, "int");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_IDENTIFIER);
    ck_assert_str_eq(tok->lexeme, "main");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_SEPARATOR_LEFT_PARENTHESIS);
    ck_assert_str_eq(tok->lexeme, "(");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_SEPARATOR_RIGHT_PARENTHESIS);
    ck_assert_str_eq(tok->lexeme, ")");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_SEPARATOR_LEFT_BRACE);
    ck_assert_str_eq(tok->lexeme, "{");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_KEYWORD_RETURN);
    ck_assert_str_eq(tok->lexeme, "return");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_CONSTANT_INT);
    ck_assert_str_eq(tok->lexeme, "0");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_SEPARATOR_SEMICOLON);
    ck_assert_str_eq(tok->lexeme, ";");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_SEPARATOR_RIGHT_BRACE);
    ck_assert_str_eq(tok->lexeme, "}");
    token_free(tok);
    tok = list_entry_grab(tokens, token_t, list);
    ck_assert_int_eq(tok->type, TOK_EOF);
    token_free(tok);
}
END_TEST

Suite *lexer_suite(void)
{
    Suite *s;
    TCase *lexer;

    s = suite_create("\n Lexical Analysis Tests");

    lexer = tcase_create("Lexer");
    tcase_add_test(lexer, test_lex_numbers);
    tcase_add_test(lexer, test_lex_keywords);
    tcase_add_test(lexer, test_lex_operators);
    tcase_add_test(lexer, test_lex_strings_and_chars);
    tcase_add_test(lexer, test_lex_whitespaces_and_comments);
    suite_add_tcase(s, lexer);

    return s;
}

/**********************************************************************
 * Parser tests
 **********************************************************************/
START_TEST(test_parser_wrong_assign)
{
    int ck = check_cmd("./tc -s 'int main(){1 = x;}' 2>&1", "Identifier expected, but got 1");
    ck_assert_int_eq(ck, 1);
}
END_TEST

START_TEST(test_parser_if_while_stmt)
{
    char *prog = "int main(){if(1) while(0) return 0; else return 1;}";
    struct list_head *tokens = lex(prog);
    cast_node_t* root = parse(tokens);

    ck_assert_ptr_ne(root, NULL);
    ck_assert_int_eq(root->type, CAST_PROGRAM);
    ck_assert_int_eq(list_size(&root->program.declarations), 1);

    cast_node_t *d = list_entry_grab(&root->program.declarations, cast_node_t, list);
    ck_assert_int_eq(d->type, CAST_FUN_DECLARATION);
    ck_assert_int_eq(list_size(&d->fun_declaration.compound_stmt->compound_stmt.stmts), 1);
    cast_node_t *s = list_entry_grab(&d->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
    ck_assert_int_eq(s->type, CAST_IF_STMT);
    ck_assert_int_eq(s->if_stmt.expr->type, CAST_NUMBER);
    ck_assert_int_eq(s->if_stmt.expr->expr.num, 1);
    ck_assert_int_eq(s->if_stmt.if_stmt->type, CAST_WHILE_STMT);
    ck_assert_int_eq(s->if_stmt.if_stmt->while_stmt.expr->type, CAST_NUMBER);
    ck_assert_int_eq(s->if_stmt.if_stmt->while_stmt.expr->expr.num, 0);
    ck_assert_int_eq(s->if_stmt.if_stmt->while_stmt.stmt->type, CAST_RETURN_STMT);
    ck_assert_int_eq(s->if_stmt.if_stmt->while_stmt.stmt->return_stmt.expr->type, CAST_NUMBER);
    ck_assert_int_eq(s->if_stmt.if_stmt->while_stmt.stmt->return_stmt.expr->expr.num, 0);
    ck_assert_int_eq(s->if_stmt.else_stmt->type, CAST_RETURN_STMT);
    ck_assert_int_eq(s->if_stmt.else_stmt->return_stmt.expr->type, CAST_NUMBER);
    ck_assert_int_eq(s->if_stmt.else_stmt->return_stmt.expr->expr.num, 1);
}
END_TEST

START_TEST(test_parser_expr)
{
   char *prog = "int x, y; int main(){x = 1; y = 1 + x; x = 1*2 + 1; y = 1 * (1+y); return x;}";
   struct list_head *tokens = lex(prog);
   cast_node_t* root = parse(tokens);

   ck_assert_ptr_ne(root, NULL);
   ck_assert_int_eq(root->type, CAST_PROGRAM);
   ck_assert_int_eq(list_size(&root->program.declarations), 2);

   cast_node_t *d = list_entry_grab(&root->program.declarations, cast_node_t, list);
   d = list_entry_grab(&root->program.declarations, cast_node_t, list);
   ck_assert_int_eq(d->type, CAST_FUN_DECLARATION);

   // Check the first statement x = 1
   cast_node_t *stmt = list_entry_grab(&d->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
   ck_assert_int_eq(stmt->type, CAST_ASSIGN_STMT);
   ck_assert_str_eq(stmt->assign_stmt.identifier, "x");
   ck_assert_int_eq(stmt->assign_stmt.expr->type, CAST_NUMBER);
   ck_assert_int_eq(stmt->assign_stmt.expr->expr.num, 1);
   // Check the second statement y = 1 + x
   stmt = list_entry_grab(&d->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
   ck_assert_int_eq(stmt->type, CAST_ASSIGN_STMT);
   ck_assert_str_eq(stmt->assign_stmt.identifier, "y");
   ck_assert_int_eq(stmt->assign_stmt.expr->type, CAST_SIMPLE_EXPR);
   ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.type, TOK_OPERATOR_ADD);
   ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->type, CAST_NUMBER);
   ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->expr.num, 1);
   ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.right->type, CAST_IDENTIFIER);
   ck_assert_str_eq(stmt->assign_stmt.expr->expr.op.right->expr.identifier, "x");
    // Check the third statement x = 1*2 + 1;
    stmt = list_entry_grab(&d->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
    ck_assert_int_eq(stmt->type, CAST_ASSIGN_STMT);
    ck_assert_str_eq(stmt->assign_stmt.identifier, "x");
    ck_assert_int_eq(stmt->assign_stmt.expr->type, CAST_SIMPLE_EXPR);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.type, TOK_OPERATOR_ADD);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->type, CAST_TERM);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->expr.op.type, TOK_OPERATOR_MUL);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->expr.op.left->type, CAST_NUMBER);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->expr.op.left->expr.num, 1);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->expr.op.right->type, CAST_NUMBER);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->expr.op.right->expr.num, 2);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.right->type, CAST_NUMBER);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.right->expr.num, 1);
    // Check the fourth statement y = 1 * (1+y);
    stmt = list_entry_grab(&d->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
    ck_assert_int_eq(stmt->type, CAST_ASSIGN_STMT);
    ck_assert_str_eq(stmt->assign_stmt.identifier, "y");
    ck_assert_int_eq(stmt->assign_stmt.expr->type, CAST_TERM);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.type, TOK_OPERATOR_MUL);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->type, CAST_NUMBER);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.left->expr.num, 1);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.right->type, CAST_SIMPLE_EXPR);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.right->expr.op.type, TOK_OPERATOR_ADD);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.right->expr.op.left->type, CAST_NUMBER);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.right->expr.op.left->expr.num, 1);
    ck_assert_int_eq(stmt->assign_stmt.expr->expr.op.right->expr.op.right->type, CAST_IDENTIFIER);
    ck_assert_str_eq(stmt->assign_stmt.expr->expr.op.right->expr.op.right->expr.identifier, "y");
    // Check the fifth statement return x;
    stmt = list_entry_grab(&d->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
    ck_assert_int_eq(stmt->type, CAST_RETURN_STMT);
    ck_assert_int_eq(stmt->return_stmt.expr->type, CAST_IDENTIFIER);
    ck_assert_str_eq(stmt->return_stmt.expr->expr.identifier, "x");
}
END_TEST

START_TEST(test_parser_empty_stmt)
{
    char *prog = ";int y;;int add(){;;};;int main(){;int x;;x = 2;y = 2;return x;};";
    struct list_head *tokens = lex(prog);
    cast_node_t* root = parse(tokens);
    token_t *tok;

    ck_assert_ptr_ne(root, NULL);
    ck_assert_int_eq(root->type, CAST_PROGRAM);
    ck_assert_int_eq(list_size(&root->program.declarations), 3);

    list_for_each_entry(tok, tokens, list) {
        list_del(&tok->list);
        token_free(tok);
    }
}
END_TEST

START_TEST(test_parser_fun_declaration)
{
    char *prog = "int add(int x, int y){return 0;}";
    struct list_head *tokens = lex(prog);
    cast_node_t* root = parse(tokens);
    token_t *tok;

    ck_assert_ptr_ne(root, NULL);
    ck_assert_int_eq(root->type, CAST_PROGRAM);
    ck_assert_int_eq(list_size(&root->program.declarations), 1);

    cast_node_t *func = list_entry_grab(&root->program.declarations, cast_node_t, list);
    ck_assert_int_eq(func->type, CAST_FUN_DECLARATION);
    ck_assert_int_eq(func->fun_declaration.type, TOK_KEYWORD_INT);
    ck_assert_str_eq(func->fun_declaration.identifier, "add");
    ck_assert_int_eq(list_size(&func->fun_declaration.param_list->param_list.params), 2);

    cast_node_t *param = list_entry_grab(&func->fun_declaration.param_list->param_list.params, cast_node_t, list);
    ck_assert_int_eq(param->type, CAST_PARAM);
    ck_assert_int_eq(param->param.type, TOK_KEYWORD_INT);
    ck_assert_str_eq(param->param.param_declarator->param_declarator.identifier, "x");
    param = list_entry_grab(&func->fun_declaration.param_list->param_list.params, cast_node_t, list);
    ck_assert_int_eq(param->type, CAST_PARAM);
    ck_assert_int_eq(param->param.type, TOK_KEYWORD_INT);
    ck_assert_str_eq(param->param.param_declarator->param_declarator.identifier, "y");
    ck_assert_int_eq(list_size(&func->fun_declaration.compound_stmt->compound_stmt.stmts), 1);
    cast_node_t *stmt = list_entry_grab(&func->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
    ck_assert_int_eq(stmt->type, CAST_RETURN_STMT);
    ck_assert_int_eq(stmt->return_stmt.expr->type, CAST_NUMBER);
    ck_assert_int_eq(stmt->return_stmt.expr->expr.num, 0);

    list_for_each_entry(tok, tokens, list) {
        list_del(&tok->list);
        token_free(tok);
    }
}
END_TEST

START_TEST(test_parser_declaration)
{
    char *prog = "int x, y;int z; int main(int x);";
    struct list_head *tokens = lex(prog);
    token_t *tok;
    cast_node_t* root = parse(tokens);

    ck_assert_ptr_ne(root, NULL);
    ck_assert_int_eq(root->type, CAST_PROGRAM);
    ck_assert_int_eq(list_size(&root->program.declarations), 3);

    cast_node_t *d = list_entry_grab(&root->program.declarations, cast_node_t, list);
    ck_assert_int_eq(d->type, CAST_VAR_DECLARATION);
    ck_assert_int_eq(d->var_declaration.type, TOK_KEYWORD_INT);
    cast_node_t *i = list_entry_grab(&d->var_declaration.var_declarator_list->var_declarator_list.var_declarators, cast_node_t, list);
    ck_assert_str_eq(i->var_declarator.identifier, "x");
    i = list_entry_grab(&d->var_declaration.var_declarator_list->var_declarator_list.var_declarators, cast_node_t, list);
    ck_assert_str_eq(i->var_declarator.identifier, "y");

    d = list_entry_grab(&root->program.declarations, cast_node_t, list);
    ck_assert_int_eq(d->type, CAST_VAR_DECLARATION);
    ck_assert_int_eq(d->var_declaration.type, TOK_KEYWORD_INT);
    i = list_entry_grab(&d->var_declaration.var_declarator_list->var_declarator_list.var_declarators, cast_node_t, list);
    ck_assert_str_eq(i->var_declarator.identifier, "z");

    d = list_entry_grab(&root->program.declarations, cast_node_t, list);
    ck_assert_int_eq(d->type, CAST_FUN_DECLARATION);
    ck_assert_int_eq(d->fun_declaration.type, TOK_KEYWORD_INT);
    ck_assert_str_eq(d->fun_declaration.identifier, "main");
    ck_assert_int_eq(list_size(&d->fun_declaration.param_list->param_list.params), 1);
    i = list_entry_grab(&d->fun_declaration.param_list->param_list.params, cast_node_t, list);
    ck_assert_int_eq(i->type, CAST_PARAM);
    ck_assert_int_eq(i->param.type, TOK_KEYWORD_INT);
    ck_assert_str_eq(i->param.param_declarator->param_declarator.identifier, "x");
    ck_assert_ptr_eq(d->fun_declaration.compound_stmt, NULL);

    list_for_each_entry(tok, tokens, list) {
        list_del(&tok->list);
        token_free(tok);
    }
}
END_TEST

Suite *parser_suite(void)
{
    Suite *s;
    TCase *parser;

    s = suite_create("Syntax Analysis Tests");

    parser = tcase_create("Parser");
    tcase_add_test(parser, test_parser_empty_stmt);
    tcase_add_test(parser, test_parser_declaration);
    tcase_add_test(parser, test_parser_fun_declaration);
    tcase_add_test(parser, test_parser_expr);
    tcase_add_test(parser, test_parser_if_while_stmt);
    tcase_add_test(parser, test_parser_wrong_assign);
    suite_add_tcase(s, parser);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = lexer_suite();
    sr = srunner_create(s);
    s = parser_suite();
    srunner_add_suite(sr, s);
    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
