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
    strbuf_addstr(&ir, "\t.globl ");
    strbuf_addstr(&ir, name);
    strbuf_addstr(&ir, "\n");
	strbuf_addf(&ir, "\t.type %s, @function\n", name);
    strbuf_addstr(&ir, name);
    strbuf_addstr(&ir, ":\n");
    strbuf_addstr(&ir, "\tpushq %rbp\n");
    strbuf_addstr(&ir, "\tmovq %rsp, %rbp\n");
}

static inline void generate_function_epilogue(void)
{
    strbuf_addstr(&ir, "\tmovq %rbp, %rsp\n");
    strbuf_addstr(&ir, "\tpopq %rbp\n");
    strbuf_addstr(&ir, "\tret\n");
}

static void generate_asm(cast_node_t *node)
{
	if (!node)
		return;
    switch (node->type) {
    case CAST_PROGRAM:
        {
            cast_node_t *d;
            list_for_each_entry(d, &node->program.declarations, list) {
                generate_asm(d);
            }
        }
        break;
	case CAST_VAR_DECLARATOR:
		{
			cast_node_t *var_declarator;
			list_for_each_entry(var_declarator, &node->var_declarator_list.var_declarators, list) {
				generate_asm(var_declarator);
			}
		}
		break;
	case CAST_VAR_DECLARATOR_LIST:
		{
			cast_node_t *var_declarator;
			list_for_each_entry(var_declarator, &node->var_declarator_list.var_declarators, list) {
				generate_asm(var_declarator);
			}
		}
		break;
	case CAST_FUN_DECLARATION:
        // Generate function header
		generate_function_prologue(node->fun_declaration.identifier);
		// Generate function parameters
		generate_asm(node->fun_declaration.param_list);
        // Generate function body
        generate_asm(node->fun_declaration.compound_stmt);
        // Add return statement if none exists
        if (node->fun_declaration.type == TOK_KEYWORD_VOID)
			generate_function_epilogue();
		break;
	case CAST_PARAM_LIST:
		{
			cast_node_t *param;
			list_for_each_entry(param, &node->param_list.params, list) {
				generate_asm(param);
			}
		}
		break;
	case CAST_PARAM:
		break;
	case CAST_COMPOUND_STMT:
		{
            cast_node_t *s;
			list_for_each_entry(s, &node->compound_stmt.stmts, list) {
                generate_asm(s);
            }
        }
		break;
    case CAST_RETURN_STMT:
		// Generate return value
		generate_asm(node->return_stmt.expr);
		strbuf_addstr(&ir, "\tpopq %rax\n"); // Pop return value
		generate_function_epilogue();
		break;
	case CAST_CALL_EXPR:
    {
        // Generate code for function arguments
        cast_node_t *arg;
        int arg_count = list_size(&node->call_expr.args_list);
        list_for_each_entry_reverse(arg, &node->call_expr.args_list, list) {
            generate_asm(arg);
            if (arg_count-- <= 6) {
                // Pass the first six arguments in registers
                switch (arg_count) {
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
                        break;
                }
            } // Additional arguments are already on the stack
        }
        // Call the function
        strbuf_addf(&ir, "\tcall %s\n", node->call_expr.identifier);
        // Pop arguments off the stack
        if (arg_count > 6) {
            strbuf_addf(&ir, "\taddq $%d, %%rsp\n", (arg_count - 6) * 8);
        }
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
			generate_asm(node->expr.op.left);
			generate_asm(node->expr.op.right);
			strbuf_addstr(&ir, "\tpopq %rbx\n");		   // Pop right operand
			strbuf_addstr(&ir, "\tpopq %rax\n");		   // Pop left operand
			strbuf_addf(&ir, "\t%s %%ebx, %%eax\n", op); // operate left and right operands
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
			generate_asm(node->expr.op.left);
			generate_asm(node->expr.op.right);
			strbuf_addstr(&ir, "\tpopq %rbx\n");	// Pop right operand
			strbuf_addstr(&ir, "\tpopq %rax\n"); 	// Pop left operand
			if (strcmp(op, "idivl") == 0)
				strbuf_addstr(&ir, "\tcqo\n");	// Sign extend %rax to %rdx:%rax
			strbuf_addf(&ir, "\t%s %%ebx\n", op); // operate left and right operands
			strbuf_addstr(&ir, "\tpushq %rax\n");	// Push result
		}
		break;
	case CAST_IDENTIFIER:
		// Load the value of the identifier into %rax
        strbuf_addstr(&ir, "\tpushq %rax\n"); // Push result onto stack
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
	generate_asm(node);
	tc_debug(0, "\n%s", ir.buf);
	generate_machine_code(ir.buf);
	strbuf_release(&ir);
}