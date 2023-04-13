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
            tc_debug(0, "optimize pos = %d\n", pos);
            strbuf_remove(code, pos, strlen(str));
        }
    } while (pos != -1);
	tc_debug(1, "\n%s", code->buf);
    generate_machine_code(code->buf);
}
