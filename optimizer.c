#include "tc.h"

static void generate_machine_code(char *code)
{
	FILE *fp = fopen("a.s", "w");
	fprintf(fp, "%s", code);
	fclose(fp);
	system("gcc -o a.tc a.s");
	remove("a.s");
}

void optimize_code(struct strbuf *code)
{
    int pos;
    do { // remove useless paired pushq/popq with same %rax
        char *str = "\tpushq %rax\n\tpopq %rax\n";
        pos = strbuf_findstr(code, str);
        if (pos != -1) {
            tc_debug(0, "optimize[1] pos = %d\n", pos);
            strbuf_remove(code, pos, strlen(str));
        }
    } while (pos != -1);
    do {// replace pushq/popq with movq
        char *str = "\tpushq %rax\n\tpopq";
        char *new = "\tmovq %rax,";
        pos = strbuf_findstr(code, str);
        if (pos != -1) {
            tc_debug(0, "optimize[2] pos = %d\n", pos);
            strbuf_splice(code, pos, strlen(str), new, strlen(new));
        }
    } while (pos != -1);
    do {// merge double mov
        char *str = ", %eax\n\tmovl %eax";
        pos = strbuf_findstr(code, str);
        if (pos != -1) {
            tc_debug(0, "optimize[3] pos = %d\n", pos);
            strbuf_remove(code, pos, strlen(str));
        }
    } while (pos != -1);
	tc_debug(1, "\n%s", code->buf);
    generate_machine_code(code->buf);
}
