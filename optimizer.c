#include "tc.h"

void optimize_code(struct strbuf *code)
{
    int pos, start = 0;
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
        pos = strbuf_findstr_pos(code, str, start);
        if (pos != -1) {
            int ipos = pos;
            if (strncmp(code->buf + ipos - 1, ")", 1) == 0)
                goto forward; // first oprand is a memory address, just skip
            while (!isspace(code->buf[ipos--]))
                ; // skip first oprand of movl xxx, %eax
            while (!isspace(code->buf[--ipos]))
                ; // get the pos of instruction
            if (strncmp(code->buf + ipos + 1, "mov", 3) == 0) {
                tc_debug(0, "optimize[3] pos = %d\n", pos);
                strbuf_remove(code, pos, strlen(str));
            }
forward:
            start = pos + strlen(str);
        }
    } while (pos != -1);
	tc_debug(1, "The assembly code:\n%s", code->buf);
}
