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

static inline void strbuf_release(struct strbuf *sb)
{
	free(sb->buf);
	memset(sb, 0, sizeof(*sb));
}

static struct strbuf ir = STRBUF_INIT;

static inline void generate_function_prologue(const char *name)
{
    strbuf_addf(&ir, "\n\t.globl %s\n", name);
    strbuf_addstr(&ir, "\t.text\n");
	strbuf_addf(&ir, "\t.type %s, @function\n", name);
    strbuf_addstr(&ir, name);
    strbuf_addstr(&ir, ":\n");
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
            if (sym->is_global) {
                strbuf_addf(&ir, "\n\t.globl %s\n", sym->name);
                strbuf_addf(&ir, "\t.align 4\n");
                strbuf_addf(&ir, "\t.type %s, @object\n", sym->name);
                strbuf_addf(&ir, "\t.size %s, 4\n", sym->name);
                if (node->var_declarator.expr) {
                    strbuf_addstr(&ir, "\t.data\n");
                    strbuf_addf(&ir, "%s:\n", sym->name);
                    strbuf_addf(&ir, "\t.long %d\n", node->var_declarator.expr->expr.num);
                } else {
                    strbuf_addstr(&ir, "\t.bss\n");
                    strbuf_addf(&ir, "%s:\n", sym->name);
                    strbuf_addf(&ir, "\t.zero 4\n");
                }
             }
       }
       break;
	case CAST_FUN_DECLARATION:
        // Generate function header
		generate_function_prologue(node->fun_declaration.identifier);
		// Generate function parameters
		generate_asm(node->fun_declaration.param_list, node->fun_declaration.symbol_table);
        // Generate function body
        generate_asm(node->fun_declaration.compound_stmt, node->fun_declaration.symbol_table);
        // Add return statement if none exists
        if (node->fun_declaration.type == TOK_KEYWORD_VOID)
			generate_function_epilogue();
		break;
	case CAST_PARAM_LIST:
		{
			cast_node_t *param;
			int idx = 0;
			list_for_each_entry(param, &node->param_list.params, list) {
				param->param.index = ++idx;
				generate_asm(param, symtab);
			}
            strbuf_addf(&ir, "\tsubq $%d, %%rsp\n", 4 * idx);
		}
		break;
	case CAST_PARAM:
		// move parameter from register or stack to local stack frame
    {
        symbol_t *sym = symbol_table_lookup(symtab, node->param.identifier, 0);
        sym->offset = -4 * node->param.index;
        switch (node->param.index) {
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
    case CAST_RETURN_STMT:
		// Generate return value
		generate_asm(node->return_stmt.expr, symtab);
		strbuf_addstr(&ir, "\tpopq %rax\n"); // Pop return value
		generate_function_epilogue();
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
			// Call the function
            strbuf_addf(&ir, "\tcall %s\n", node->call_expr.identifier);
            // Pop arguments off the stack if there are more than 6
            // Push the return value onto the stack
            strbuf_addstr(&ir, "\tpushq %rax\n");
        }
        break;
	case CAST_IF_STMT:
		break;
	case CAST_EXPR:
		break;
	case CAST_RELATIONAL_EXPR:
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
            if (sym->is_global)
                strbuf_addf(&ir, "\tmovl %s(%%rip), %%eax\n", sym->name);
            else
                strbuf_addf(&ir, "\tmovl %d(%%rbp), %%eax\n", sym->offset);
            strbuf_addstr(&ir, "\tpushq %rax\n"); // Push result onto stack
        }
		break;
	case CAST_NUMBER:
		// Generate code for number and push it on the stack
		strbuf_addf(&ir, "\tmovl $%d, %%eax\n", node->expr.num);
		strbuf_addstr(&ir, "\tpushq %rax\n");
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
	tc_debug(0, "\n%s", ir.buf);
	generate_machine_code(ir.buf);
	strbuf_release(&ir);
}
