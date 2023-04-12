#include "tc.h"

struct strbuf {
	size_t alloc;
	size_t len;
	int eof;
	char *buf;
};

#define alloc_nr(x) (((x)+16)*3/2)

#define ALLOC_GROW(x, nr, alloc) \
	do { \
		if ((nr) > alloc) { \
			if (alloc_nr(alloc) < (nr)) \
				alloc = (nr); \
			else \
				alloc = alloc_nr(alloc); \
			x = realloc((x), alloc * sizeof(*(x))); \
		} \
	} while (0)

#define STRBUF_INIT  { 0, 0, 0, NULL }

static void strbuf_grow(struct strbuf *sb, size_t extra)
{
	if (sb->len + extra + 1 <= sb->len)
    	panic("you want to use way too much memory");
	ALLOC_GROW(sb->buf, sb->len + extra + 1, sb->alloc);
}

static inline void strbuf_setlen(struct strbuf *sb, size_t len)
{
	sb->len = len;
	sb->buf[len] = '\0';
}

static inline size_t strbuf_avail(struct strbuf *sb)
{
	return sb->alloc ? sb->alloc - sb->len - 1 : 0;
}

static void strbuf_add(struct strbuf *sb, const void *data, size_t len)
{
	strbuf_grow(sb, len);
	memcpy(sb->buf + sb->len, data, len);
	strbuf_setlen(sb, sb->len + len);
}

static inline void strbuf_addstr(struct strbuf *sb, const char *s)
{
	strbuf_add(sb, s, strlen(s));
}

static void strbuf_addf(struct strbuf *sb, const char *fmt, ...)
{
	int len;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf(sb->buf + sb->len, sb->alloc - sb->len, fmt, ap);
	va_end(ap);
	if (len < 0)
		len = 0;
	if (len > strbuf_avail(sb)) {
		strbuf_grow(sb, len);
		va_start(ap, fmt);
		len = vsnprintf(sb->buf + sb->len, sb->alloc - sb->len, fmt, ap);
		va_end(ap);
	}
	strbuf_setlen(sb, sb->len + len);
}

static void strbuf_vinsertf(struct strbuf *sb, size_t pos, const char *fmt, va_list ap)
{
    int len, len2;
    char save;
    va_list cp;

    if (pos > sb->len)
        panic("`pos' is too far after the end of the buffer");
    va_copy(cp, ap);
    len = vsnprintf(sb->buf + sb->len, 0, fmt, cp);
    va_end(cp);
    if (len < 0)
        panic("your vsnprintf is broken (returned %d)", len);
    if (!len)
        return; /* nothing to do */
    strbuf_grow(sb, len);
    memmove(sb->buf + pos + len, sb->buf + pos, sb->len - pos);
    /* vsnprintf() will append a NUL, overwriting one of our characters */
    save = sb->buf[pos + len];
    len2 = vsnprintf(sb->buf + pos, len + 1, fmt, ap);
    sb->buf[pos + len] = save;
    if (len2 != len)
        panic("your vsnprintf is broken (returns inconsistent lengths)");
    strbuf_setlen(sb, sb->len + len);
}

static inline void strbuf_insertf(struct strbuf *sb, size_t pos, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    strbuf_vinsertf(sb, pos, fmt, ap);
    va_end(ap);
}

static inline void strbuf_head_addf(struct strbuf *sb, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    strbuf_vinsertf(sb, 0, fmt, ap);
    va_end(ap);
}

static inline void strbuf_release(struct strbuf *sb)
{
	free(sb->buf);
	memset(sb, 0, sizeof(*sb));
}

// stack offset of a variable or parameter
static inline int to_offset(int index)
{
    return index * -4;
}

static struct strbuf ir = STRBUF_INIT;

static inline void generate_function_prologue(const char *name)
{
    strbuf_addf(&ir, "\n\t.globl %s\n", name);
    strbuf_addstr(&ir, "\t.text\n");
	strbuf_addf(&ir, "\t.type %s, @function\n", name);
    strbuf_addstr(&ir, name);
    strbuf_addstr(&ir, ":\n");
    strbuf_addstr(&ir, "\tendbr64\n");
    strbuf_addstr(&ir, "\tpushq %rbp\n");
    strbuf_addstr(&ir, "\tmovq %rsp, %rbp\n");
}

static inline void generate_function_epilogue(void)
{
    strbuf_addstr(&ir, "\tleave\n"); // restore stack pointer
    strbuf_addstr(&ir, "\tret\n");
}

static void generate_asm(cast_node_t *node, symbol_table_t *symtab)
{
    static int label_count = 0;
    if (!node)
        return;
    switch (node->type) {
    case CAST_PROGRAM:
        {
            cast_node_t *d;
            list_for_each_entry(d, &node->program.declarations, list) {
                generate_asm(d, symtab);
            }
        }
        break;
    case CAST_VAR_DECLARATION:
        generate_asm(node->var_declaration.var_declarator_list, symtab);
        break;
    case CAST_VAR_DECLARATOR_LIST:
        {
            cast_node_t *var_declarator;
            list_for_each_entry(var_declarator, &node->var_declarator_list.var_declarators, list) {
                generate_asm(var_declarator, symtab);
            }
        }
        break;
    case CAST_VAR_DECLARATOR:
        {
            symbol_t *sym = symbol_table_lookup(symtab, node->var_declarator.identifier, 0);
            if (sym->index == 0) {// global variable
                strbuf_addf(&ir, "\n\t.globl %s\n", sym->name);
                strbuf_addf(&ir, "\t.align 4\n"); // align to 4 bytes
                strbuf_addf(&ir, "\t.type %s, @object\n", sym->name); // @object is for data
                strbuf_addf(&ir, "\t.size %s, 4\n", sym->name); // size in bytes
                if (node->var_declarator.expr) {
                    strbuf_addstr(&ir, "\t.data\n"); // data section
                    strbuf_addf(&ir, "%s:\n", sym->name);
                    strbuf_addf(&ir, "\t.long %d\n", node->var_declarator.expr->expr.num);
                } else {
                    strbuf_addstr(&ir, "\t.bss\n"); // uninitialized data section
                    strbuf_addf(&ir, "%s:\n", sym->name);
                    strbuf_addf(&ir, "\t.zero 4\n"); // zero out 4 bytes
                }
            } else {
                tc_debug(0, "local variable %s, index %d\n", sym->name, sym->index);
                if (node->var_declarator.expr) {
                    // for local variables, we support real expressions.
                    generate_asm(node->var_declarator.expr, symtab);
                    strbuf_addstr(&ir, "\tpopq %rax\n"); //get the value of the expression
                    strbuf_addf(&ir, "\tmovl %%eax, %d(%%rbp)\n", to_offset(sym->index)); // initialize the variable
                }
            }
        }
        break;
    case CAST_FUN_DECLARATION:
        {
            symbol_t *sym = symbol_table_lookup(symtab, node->fun_declaration.identifier, 0);
            // Generate function header
            generate_function_prologue(node->fun_declaration.identifier);
            // Allocate space for local variables and round up to 16 bytes to keep stack 16 bytes-aligned
            // see https://stackoverflow.com/questions/49391001/why-does-the-x86-64-amd64-system-v-abi-mandate-a-16-byte-stack-alignment
            #define ROUND_UP_16(x) (((x) + 15) & ~15)
            if (sym->var_count+sym->arg_count > 0)
                strbuf_addf(&ir, "\tsubq $%d, %%rsp\n", ROUND_UP_16((sym->var_count+sym->arg_count) * 4));
            // Generate function parameters
            generate_asm(node->fun_declaration.param_list, node->fun_declaration.symbol_table);
            // Generate function body
            generate_asm(node->fun_declaration.compound_stmt, node->fun_declaration.symbol_table);
            // Add return statement if none exists
            cast_node_t *last = list_last_entry(&node->fun_declaration.compound_stmt->compound_stmt.stmts, cast_node_t, list);
            if (last && last->type != CAST_RETURN_STMT)
                generate_function_epilogue();
        }
        break;
    case CAST_PARAM_LIST:
        {
            cast_node_t *param;
            list_for_each_entry(param, &node->param_list.params, list) {
                generate_asm(param, symtab);
            }
        }
        break;
    case CAST_PARAM:
        // move parameter from register or stack to local stack frame
        {
            symbol_t *sym = symbol_table_lookup(symtab, node->param.identifier, 0);
            tc_debug(0, "local param %s, index %d\n", sym->name, sym->index);
            switch (sym->index) {
                case 1:
                    // 1st parameter is in %edi, move it to -4(%rbp)
                    strbuf_addstr(&ir, "\tmovl %edi, -4(%rbp)\n");
                    break;
                case 2:
                    // 2nd parameter is in %esi, move it to -8(%rbp)
                    strbuf_addstr(&ir, "\tmovl %esi, -8(%rbp)\n");
                    break;
                case 3:
                    // 3rd parameter is in %edx, move it to -12(%rbp)
                    strbuf_addstr(&ir, "\tmovl %edx, -12(%rbp)\n");
                case 4:
                    // 4th parameter is in %ecx, move it to -16(%rbp)
                    strbuf_addstr(&ir, "\tmovl %ecx, -16(%rbp)\n");
                    break;
                case 5:
                    // 5th parameter is in %r8d, move it to -20(%rbp)
                    strbuf_addstr(&ir, "\tmovl %r8d, -20(%rbp)\n");
                    break;
                case 6:
                    // 6th parameter is in %r9d, move it to -24(%rbp)
                    strbuf_addstr(&ir, "\tmovl %r9d, -24(%rbp)\n");
                    break;
                default:
                    panic("FIX ME:too many parameters\n");
            }
        }
        break;
    case CAST_COMPOUND_STMT:
        {
            cast_node_t *s;
            list_for_each_entry(s, &node->compound_stmt.stmts, list) {
                generate_asm(s, symtab);
            }
        }
        break;
    case CAST_ASSIGN_STMT:
        {
            symbol_t *sym = symbol_table_lookup(symtab, node->assign_stmt.identifier, 1);
            generate_asm(node->assign_stmt.expr, symtab);
            strbuf_addstr(&ir, "\tpopq %rax\n"); // Pop value of expression
            if (sym->index == 0)
                strbuf_addf(&ir, "\tmovl %%eax, %s(%%rip)\n", sym->name); // Store value in variable
            else
                strbuf_addf(&ir, "\tmovl %%eax, %d(%%rbp)\n", to_offset(sym->index)); // Store value in variable
        }
        break;
    case CAST_RETURN_STMT:
        // Generate return value
        generate_asm(node->return_stmt.expr, symtab);
        strbuf_addstr(&ir, "\tpopq %rax\n"); // Pop return value
        generate_function_epilogue();
        break;
    case CAST_WHILE_STMT: {
        int start_label = label_count++;
        int end_label = label_count++;
        // Generate code for condition
        strbuf_addf(&ir, ".L%d:\n", start_label);
        generate_asm(node->while_stmt.expr, symtab);
        strbuf_addstr(&ir, "\tpopq %rax\n");       // Pop condition result
        strbuf_addstr(&ir, "\ttest %rax, %rax\n"); // Test condition
        strbuf_addf(&ir, "\tje .L%d\n", end_label); // Jump to end of while loop if condition is false
        // Generate code for body
        generate_asm(node->while_stmt.stmt, symtab);
        strbuf_addf(&ir, "\tjmp .L%d\n", start_label); // Jump to start of while loop
        // Generate code for end of while loop
        strbuf_addf(&ir, ".L%d:\n", end_label);
        }
        break;
    case CAST_CALL_EXPR:
        {
            // Generate code for function arguments
            cast_node_t *arg;
            int arg_count = list_size(&node->call_expr.args_list);
            if (arg_count > 6)
                panic("FIX ME:too many arguments\n");
            // Evaluate arguments in reverse order
            list_for_each_entry_reverse(arg, &node->call_expr.args_list, list) {
                generate_asm(arg, symtab);
            }
            // Pass the first six arguments in registers
            for (int i = 1; i <= arg_count; i++) {
                switch (i) {
                    case 1:
                        strbuf_addstr(&ir, "\tpopq %rdi\n");
                        break;
                    case 2:
                        strbuf_addstr(&ir, "\tpopq %rsi\n");
                        break;
                    case 3:
                        strbuf_addstr(&ir, "\tpopq %rdx\n");
                        break;
                    case 4:
                        strbuf_addstr(&ir, "\tpopq %rcx\n");
                        break;
                    case 5:
                        strbuf_addstr(&ir, "\tpopq %r8\n");
                        break;
                    case 6:
                        strbuf_addstr(&ir, "\tpopq %r9\n");
                    default:
                        break;
                }
            }
            // we need to zero out %eax before calling a variadic function
            // see https://stackoverflow.com/questions/6212665/why-is-eax-zeroed-before-a-call-to-printf
            strbuf_addstr(&ir, "\tmovl $0, %eax\n");
            // Call the function
            strbuf_addf(&ir, "\tcall %s\n", node->call_expr.identifier);
            // Pop arguments off the stack if there are more than 6
            // Push the return value onto the stack
            strbuf_addstr(&ir, "\tpushq %rax\n");
        }
        break;
    case CAST_IF_STMT:
        {
            int else_label = label_count++;
            int end_label = label_count++;
            // Generate code for condition
            generate_asm(node->if_stmt.expr, symtab);
            strbuf_addstr(&ir, "\tpopq %rax\n");       // Pop condition value
            strbuf_addstr(&ir, "\ttest %rax, %rax\n"); // Test condition
            // Generate code for then branch
            strbuf_addf(&ir, "\tje .L%d\n", else_label); // Jump to else branch if condition is false
            generate_asm(node->if_stmt.if_stmt, symtab);
            strbuf_addf(&ir, "\tjmp .L%d\n", end_label); // Jump to end of if statement
            // Generate code for else branch
            strbuf_addf(&ir, ".L%d:\n", else_label);
            generate_asm(node->if_stmt.else_stmt, symtab);
            // Generate code for end of if statement
            strbuf_addf(&ir, ".L%d:\n", end_label);
        }
        break;
    case CAST_EXPR:
        break;
    case CAST_RELATIONAL_EXPR: {
        // Generate code for left and right operands
        generate_asm(node->expr.op.left, symtab);
        generate_asm(node->expr.op.right, symtab);
        strbuf_addstr(&ir, "\tpopq %rcx\n"); // Pop right operand
        strbuf_addstr(&ir, "\tpopq %rax\n"); // Pop left operand
        // Compare left and right operands
        switch (node->expr.op.type) {
        case TOK_OPERATOR_LESS_THAN:
            strbuf_addstr(&ir, "\tcmpq %rcx, %rax\n"); // Compare left and right operands
            strbuf_addstr(&ir, "\tsetl %al\n"); // Set %al to 1 if left operand is less than right operand
            break;
        case TOK_OPERATOR_GREATER_THAN:
            strbuf_addstr(&ir, "\tcmpq %rcx, %rax\n");
            strbuf_addstr(&ir, "\tsetg %al\n"); // Set %al to 1 if left operand is greater than right operand
            break;
        case TOK_OPERATOR_LESS_THAN_OR_EQUAL_TO:
            strbuf_addstr(&ir, "\tcmpq %rcx, %rax\n");
            strbuf_addstr(&ir, "\tsetle %al\n"); // Set %al to 1 if left operand is less than or equal to right operand
            break;
        case TOK_OPERATOR_GREATER_THAN_OR_EQUAL_TO:
            strbuf_addstr(&ir, "\tcmpq %rcx, %rax\n");
            strbuf_addstr(&ir, "\tsetge %al\n"); // Set %al to 1 if left operand is greater than or equal to right operand
            break;
        case TOK_OPERATOR_EQUAL:
            strbuf_addstr(&ir, "\tcmpq %rcx, %rax\n");
            strbuf_addstr(&ir, "\tsete %al\n"); // Set %al to 1 if left operand is equal to right operand
            break;
        case TOK_OPERATOR_NOT_EQUAL:
            strbuf_addstr(&ir, "\tcmpq %rcx, %rax\n");
            strbuf_addstr(&ir, "\tsetne %al\n"); // Set %al to 1 if left operand is not equal to right operand
            break;
        default:
            panic("Invalid relational operator\n");
            break;
        }
        // Push result of comparison onto stack
        strbuf_addstr(&ir, "\tmovzbl %al, %eax\n"); // Zero extend %al to %eax
        strbuf_addstr(&ir, "\tpushq %rax\n");
    }
        break;
    case CAST_SIMPLE_EXPR:
        {
            char *op;
            if (node->expr.op.type == TOK_OPERATOR_ADD)
                op = "addl";
            else if (node->expr.op.type == TOK_OPERATOR_SUB)
                op = "subl";
            else
                panic("Unknown operator type %d\n", node->expr.op.type);
            generate_asm(node->expr.op.left, symtab);
            generate_asm(node->expr.op.right, symtab);
            strbuf_addstr(&ir, "\tpopq %r10\n");		   // Pop right operand
            strbuf_addstr(&ir, "\tpopq %rax\n");		   // Pop left operand
            strbuf_addf(&ir, "\t%s %%r10d, %%eax\n", op); // operate left and right operands
            strbuf_addstr(&ir, "\tpushq %rax\n");		   // Push result
        }
        break;
    case CAST_TERM:
        {
            char *op;
            if (node->expr.op.type == TOK_OPERATOR_MUL)
                op = "imull";
            else if (node->expr.op.type == TOK_OPERATOR_DIV)
                op = "idivl";
            else
                panic("Unknown operator type %d\n", node->expr.op.type);
            generate_asm(node->expr.op.left, symtab);
            generate_asm(node->expr.op.right, symtab);
            strbuf_addstr(&ir, "\tpopq %r10\n");	// Pop right operand
            strbuf_addstr(&ir, "\tpopq %rax\n"); 	// Pop left operand
            if (strcmp(op, "idivl") == 0)
                strbuf_addstr(&ir, "\tcqo\n");	// Sign extend %rax to %rdx:%rax
            strbuf_addf(&ir, "\t%s %%r10d\n", op); // operate left and right operands
            strbuf_addstr(&ir, "\tpushq %rax\n");	// Push result
        }
        break;
    case CAST_IDENTIFIER:
        {
            symbol_t *sym = symbol_table_lookup(symtab, node->expr.identifier, 1);
            // Load the value of the identifier into %rax
            if (sym->index == 0)
                strbuf_addf(&ir, "\tmovl %s(%%rip), %%eax\n", sym->name);
            else
                strbuf_addf(&ir, "\tmovl %d(%%rbp), %%eax\n", to_offset(sym->index));
            strbuf_addstr(&ir, "\tpushq %rax\n"); // Push result onto stack
        }
        break;
    case CAST_NUMBER:
        // Generate code for number and push it on the stack
        strbuf_addf(&ir, "\tmovl $%d, %%eax\n", node->expr.num);
        strbuf_addstr(&ir, "\tpushq %rax\n");
        break;
    case CAST_STRING:
        {
        // Generate code for string and push it on the stack
        static int string_count = 0;
        strbuf_head_addf(&ir, "\t.section\t.rodata\n.LC%d:\n\t.string %s\n",
                         string_count, node->expr.string);
        strbuf_addf(&ir, "\tleaq .LC%d(%%rip), %%rax\n", string_count);
        strbuf_addf(&ir, "\tpushq %%rax\n");
        string_count++;
        }
        break;
    default:
        break;
    }
    return;
}

static void generate_machine_code(char *code)
{
	FILE *fp = fopen("a.s", "w");
	fprintf(fp, "%s", code);
	fclose(fp);
	system("gcc -o a.tc a.s");
	remove("a.s");
}

void generate_code(cast_node_t *node)
{
	generate_asm(node, node->program.symbol_table);
	tc_debug(1, "\n%s", ir.buf);
	generate_machine_code(ir.buf);
	strbuf_release(&ir);
}
