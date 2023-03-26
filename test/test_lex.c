#include <stdlib.h>
#include <check.h>
#include "../tc.h"
#include "../list.h"

START_TEST(test_lex_numbers)
{
    // TODO: Implement test case for C numbers as input string
}
END_TEST

START_TEST(test_lex_keywords)
{
    // TODO: Implement test case for C keywords as input string
}
END_TEST

START_TEST(test_lex_operators)
{
    // TODO: Implement test case for C operators as input string
}
END_TEST

START_TEST(test_lex_strings_and_chars)
{
    // TODO: Implement test case for C strings and characters as input string
}
END_TEST

START_TEST(test_lex_whitespaces_and_comments)
{
    // TODO: Implement test case for C whitespaces and comments as input string
}
END_TEST

Suite *lex_suite(void)
{
    Suite *s;
    TCase *tc_numbers, *tc_keywords, *tc_operators, *tc_strings_and_chars,
          *tc_whitespaces_and_comments;

    s = suite_create("Lex");

    tc_numbers = tcase_create("Numbers");
    tcase_add_test(tc_numbers, test_lex_numbers);
    suite_add_tcase(s, tc_numbers);

    tc_keywords = tcase_create("Keywords");
    tcase_add_test(tc_keywords, test_lex_keywords);
    suite_add_tcase(s, tc_keywords);

    tc_operators = tcase_create("Operators");
    tcase_add_test(tc_operators, test_lex_operators);
    suite_add_tcase(s, tc_operators);

    tc_strings_and_chars = tcase_create("Strings and Chars");
    tcase_add_test(tc_strings_and_chars, test_lex_strings_and_chars);
    suite_add_tcase(s, tc_strings_and_chars);

    tc_whitespaces_and_comments = tcase_create("Whitespaces and Comments");
    tcase_add_test(tc_whitespaces_and_comments, test_lex_whitespaces_and_comments);
    suite_add_tcase(s, tc_whitespaces_and_comments);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = lex_suite();
    sr = srunner_create(s);
    srunner_run_all(sr, CK_VERBOSE);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
