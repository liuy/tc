#include <stdlib.h>
#include <check.h>
#include "../tc.h"
#include "../list.h"

static void token_free(token_t *tok)
{
    free(tok->lexeme);
    free(tok);
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
    char *prog = "int add(int x, int y){return x + y;}";
    struct list_head *tokens = lex(prog);
    cast_node_t* root = parse(tokens);

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
    ck_assert_int_eq(stmt->return_stmt.expr->type, CAST_SIMPLE_EXPR);
}
END_TEST

START_TEST(test_parser_return)
{
    char *prog = "int main()\n{\n\treturn 0;\n}";
    struct list_head *tokens = lex(prog);
    token_t *tok;
    cast_node_t* root = parse(tokens);

    ck_assert_ptr_ne(root, NULL);
    ck_assert_int_eq(root->type, CAST_PROGRAM);
    ck_assert_int_eq(list_size(&root->program.declarations), 1);

    cast_node_t *func = list_entry_grab(&root->program.declarations, cast_node_t, list);
    ck_assert_int_eq(func->type, CAST_FUN_DECLARATION);
    ck_assert_int_eq(func->fun_declaration.type, TOK_KEYWORD_INT);
    ck_assert_str_eq(func->fun_declaration.identifier, "main");
    ck_assert_ptr_eq(func->fun_declaration.param_list, NULL);
    ck_assert_ptr_ne(func->fun_declaration.compound_stmt, NULL);
    ck_assert_int_eq(func->fun_declaration.compound_stmt->type, CAST_COMPOUND_STMT);
    ck_assert_int_eq(list_size(&func->fun_declaration.compound_stmt->compound_stmt.stmts), 1);
    cast_node_t *ret = list_entry_grab(&func->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
    ck_assert_int_eq(ret->type, CAST_RETURN_STMT);
    ck_assert_int_eq(ret->return_stmt.expr->type, CAST_NUMBER);

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
    tcase_add_test(parser, test_parser_return);
    tcase_add_test(parser, test_parser_fun_declaration);
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
