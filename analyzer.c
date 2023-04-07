#include "tc.h"

static symbol_table_t *symbol_table_create(void)
{
    symbol_table_t *t = zalloc(sizeof(symbol_table_t));
    return t;
}

static void symbol_table_destroy(symbol_table_t *t)
{
    free(t); //TODO free all the table entries
}

// Lookup symbol in symbol table backwards
static symbol_t *symbol_table_lookup(symbol_table_t *t, char *name,
                                     int upward)
{
    int idx = symbol_table_hash(name);
    struct hlist_head *head = t->table + idx;
    struct hlist_node *node;

    tc_debug(0, "lookup %s in %s\n", name, t->name);
    hlist_for_each(node, head) {
        symbol_t *s = hlist_entry(node, symbol_t, list);
        if (!strcmp(s->name, name))
            return s;
    }

    if (upward && t->parent)
        return symbol_table_lookup(t->parent, name, upward);

    return NULL;
}

static void symbol_table_add(symbol_table_t *t, symbol_t *s)
{
    int idx = symbol_table_hash(s->name);
    struct hlist_head *head = t->table + idx;

    if (symbol_table_lookup(t, s->name, 0)) // just add to current scope
        panic("redeclaration of '%s'\n", s->name);
    hlist_add_head(&s->list, head);
    tc_debug(1, "<%s> %s, %s\n", t->name, token_type_to_str(s->type), s->name);
}


static void handle_var_declaration(cast_node_t *node, symbol_table_t *st)
{
    cast_node_t *var;
    list_for_each_entry(var, &node->var_declaration.var_declarator_list->var_declarator_list.var_declarators, list) {
        symbol_t *s = zalloc(sizeof(symbol_t));
        s->name = strdup(var->var_declarator.identifier);
        s->type = node->var_declaration.type;
        symbol_table_add(st, s);
    }
}

// Traverse CAST recursively in a depth-first manner
static void traverse_cast(cast_node_t *node, symbol_table_t *symtab)
{

    if (!node)
        return;

    switch (node->type) {
        case CAST_PROGRAM: {
            symbol_table_t *global;
            cast_node_t *d;
            if (symtab)
                panic("symbol table should be NULL for program node\n");
            global = symbol_table_create(); // create a global symbol table
            global->name = strdup("global");
            node->program.symbol_table = global;
            list_for_each_entry(d, &node->program.declarations, list) {
                    traverse_cast(d, global);
            }
            break;
        }
        case CAST_VAR_DECLARATION:
            handle_var_declaration(node, symtab);
            traverse_cast(node->var_declaration.var_declarator_list, symtab);
            break;
        case CAST_VAR_DECLARATOR_LIST: {
            cast_node_t *var_declarator;
            list_for_each_entry(var_declarator, &node->var_declarator_list.var_declarators, list) {
                traverse_cast(var_declarator, symtab);
            }
            break;
        }
        case CAST_VAR_DECLARATOR:
                tc_debug(0,"Var Declarator: %s\n", node->var_declarator.identifier);
                traverse_cast(node->var_declarator.expr, symtab);
            break;
        case CAST_FUN_DECLARATION: {
            symbol_t *s = zalloc(sizeof(symbol_t));
            s->name = strdup(node->fun_declaration.identifier);
            s->type = node->fun_declaration.type;
            symbol_table_add(symtab, s); // add function name to global symbol table
            symbol_table_t *local = symbol_table_create(); // create a local symbol table
            node->fun_declaration.symbol_table = local;
            local->name = strdup(node->fun_declaration.identifier);
            local->parent = symtab;
            tc_debug(0, "Fun Declaration: %s\n", node->fun_declaration.identifier);
            // Add parameters and local variables to local symbol table
            traverse_cast(node->fun_declaration.param_list, local);
            traverse_cast(node->fun_declaration.compound_stmt, local);
            break;
        }
        case CAST_PARAM_LIST: {
            cast_node_t *param;
            list_for_each_entry(param, &node->param_list.params, list) {
                traverse_cast(param, symtab);
            }
            break;
        }
        case CAST_PARAM: {
            symbol_t *s = zalloc(sizeof(symbol_t));
            s->name = strdup(node->param.param_declarator->param_declarator.identifier);
            s->type = node->param.type; //TODO: support array type
            symbol_table_add(symtab, s);
            traverse_cast(node->param.param_declarator, symtab);
            break;
        }
        case CAST_PARAM_DECLARATOR: {
            if (node->param_declarator.num)
                tc_debug(0,"Param Declarator: %s[%d]\n", node->param_declarator.identifier,
                        node->param_declarator.num);
            else
                tc_debug(0, "Param Declarator: %s\n", node->param_declarator.identifier);
            break;
        }
        case CAST_COMPOUND_STMT: {
            cast_node_t *stmt;
            list_for_each_entry(stmt, &node->compound_stmt.stmts, list) {
                if (stmt->type == CAST_COMPOUND_STMT) {
                    symbol_table_t *local = symbol_table_create();
                    stmt->compound_stmt.symbol_table = local;
                    local->name = strdup("local"); // create a nested local symbol table
                    local->parent = symtab;
                    traverse_cast(stmt, local);
                } else
                    traverse_cast(stmt, symtab);
            }
            break;
        }
        case CAST_ASSIGN_STMT: {
            symbol_t *s = symbol_table_lookup(symtab, node->assign_stmt.identifier, 1);
            if (!s)
                panic("‘%s’ undeclared (first use in %s function)\n",
                      node->assign_stmt.identifier, symtab->name);
            tc_debug(0, "Assign identifier: %s\n", node->assign_stmt.identifier);
            traverse_cast(node->assign_stmt.expr, symtab);
            break;
        }
        case CAST_RETURN_STMT:
            traverse_cast(node->return_stmt.expr, symtab);
            break;
        case CAST_WHILE_STMT:
            traverse_cast(node->while_stmt.expr, symtab);
            traverse_cast(node->while_stmt.stmt, symtab);
            break;
        case CAST_IF_STMT:
            traverse_cast(node->if_stmt.expr, symtab);
            traverse_cast(node->if_stmt.if_stmt, symtab);
            traverse_cast(node->if_stmt.else_stmt, symtab);
            break;
        case CAST_EXPR:
        case CAST_RELATIONAL_EXPR:
        case CAST_SIMPLE_EXPR:
        case CAST_TERM:
            traverse_cast(node->expr.op.left, symtab);
            traverse_cast(node->expr.op.right, symtab);
            break;
        case CAST_IDENTIFIER: {
            symbol_t *s = symbol_table_lookup(symtab, node->expr.identifier, 1);
            if (!s)
                panic("‘%s’ undeclared (first use in %s function)\n",
                      node->expr.identifier, symtab->name);
            tc_debug(0, "Identifier: %s\n", node->expr.identifier);
            break;
        }
        case CAST_NUMBER:
            tc_debug(0, "Number: %d\n", node->expr.num);
        default:
            break;
    }
    return;
}

void analyze_semantics(cast_node_t *cast_root)
{
    traverse_cast(cast_root, NULL);
}
