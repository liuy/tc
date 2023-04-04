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
static symbol_t *symbol_table_lookup(symbol_table_t *t, char *name)
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

    if (t->parent)
        return symbol_table_lookup(t->parent, name);

    return NULL;
}

static void symbol_table_add(symbol_table_t *t, symbol_t *s)
{
    int idx = symbol_table_hash(s->name);
    struct hlist_head *head = t->table + idx;

    if (symbol_table_lookup(t, s->name))
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

// Create a local symbol table for each function
static symbol_table_t *handle_fun(cast_node_t *node)
{
    symbol_table_t *local = symbol_table_create();
    cast_node_t *tmp;
    local->name = strdup(node->fun_declaration.identifier);

    // add function parameters to local symbol table
    if (node->fun_declaration.param_list)
        list_for_each_entry(tmp, &node->fun_declaration.param_list->param_list.params, list) {
            symbol_t *s = zalloc(sizeof(symbol_t));
            s->name = strdup(tmp->param.param_declarator->param_declarator.identifier);
            s->type = tmp->param.type;
            symbol_table_add(local, s);
        }

    // add local variables to local symbol table
    if (node->fun_declaration.compound_stmt)
        list_for_each_entry(tmp, &node->fun_declaration.compound_stmt->compound_stmt.stmts, list) {
            if (tmp->type == CAST_VAR_DECLARATION) {
                handle_var_declaration(tmp, local);
            }
        }
    return local;
}

static symbol_table_t *handle_declaration(cast_node_t *node, symbol_table_t *global)
{
    symbol_table_t *local = NULL;

    switch (node->type) {
        case CAST_VAR_DECLARATION:
            handle_var_declaration(node, global); // add variable name to global symbol table
            break;
        case CAST_FUN_DECLARATION: {
            symbol_t *s = zalloc(sizeof(symbol_t));
            s->name = strdup(node->fun_declaration.identifier);
            s->type = node->fun_declaration.type;
            symbol_table_add(global, s); // add function name to global symbol table
            local = handle_fun(node);
            local->parent = global;
            break;
        }
        default:
            panic("unknown declaration type %d\n", node->type);
    }
    return local;
}

// Traverse CAST recursively in a depth-first manner
static symbol_table_t *traverse_cast(cast_node_t *node, symbol_table_t *symtab)
{
    symbol_table_t *global = NULL;

    if (!node)
        goto out;

    switch (node->type) {
        case CAST_PROGRAM: {
            cast_node_t *d;
            symbol_table_t *local;
            if (symtab)
                panic("symbol table should be NULL for program node\n");
            global = symbol_table_create(); // create a global symbol table
            global->name = strdup("global");
            list_for_each_entry(d, &node->program.declarations, list) {
                local = handle_declaration(d, global);
                if (d->type == CAST_FUN_DECLARATION)
                    traverse_cast(d, local);
                else
                    traverse_cast(d, global);
            }
            break;
        }
        case CAST_VAR_DECLARATION:
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
            tc_debug(0,"Var identifier: %s\n", node->var_declarator.identifier);
            traverse_cast(node->var_declarator.num, symtab);
            break;
        case CAST_NUM:
            tc_debug(0, "Num: %d\n", node->num.value);
            break;
        case CAST_FUN_DECLARATION:
            tc_debug(0, "Fun identifier: %s\n", node->fun_declaration.identifier);
            traverse_cast(node->fun_declaration.param_list, symtab);
            traverse_cast(node->fun_declaration.compound_stmt, symtab);
            break;
        case CAST_PARAM_LIST: {
            cast_node_t *param;
            list_for_each_entry(param, &node->param_list.params, list) {
                traverse_cast(param, symtab);
            }
            break;
        }
        case CAST_PARAM:
            traverse_cast(node->param.param_declarator, symtab);
            break;
        case CAST_PARAM_DECLARATOR:
            tc_debug(0, "Param identifier: %s\n", node->param_declarator.identifier);
            traverse_cast(node->param_declarator.num, symtab);
            break;
        case CAST_COMPOUND_STMT: {
            cast_node_t *stmt;
            list_for_each_entry(stmt, &node->compound_stmt.stmts, list) {
                traverse_cast(stmt, symtab);
            }
            break;
        }
        case CAST_ASSIGN_STMT: {
            symbol_t *s = symbol_table_lookup(symtab, node->assign_stmt.identifier);
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
        case CAST_EXPR: {
            cast_node_t *relational_expr;
            list_for_each_entry(relational_expr, &node->expr.relationals, list) {
                traverse_cast(relational_expr, symtab);
            }
            break;
            }
        case CAST_RELATIONAL_EXPR:
            traverse_cast(node->relational_expr.left, symtab);
            traverse_cast(node->relational_expr.right, symtab);
            break;
        case CAST_SIMPLE_EXPR: {
            cast_node_t *term;
            list_for_each_entry(term, &node->simple_expr.terms, list) {
                traverse_cast(term, symtab);
            }
            break;
        }
        case CAST_TERM: {
            cast_node_t *factor;
            list_for_each_entry(factor, &node->term.factors, list) {
                traverse_cast(factor, symtab);
            }
            break;
        }
        case CAST_FACTOR:
            if (node->factor.identifier) {
                symbol_t *s = symbol_table_lookup(symtab, node->factor.identifier);
                if (!s)
                    panic("‘%s’ undeclared (first use in %s function)\n",
                          node->factor.identifier, symtab->name);
                tc_debug(0, "Factor identifier: %s\n", node->factor.identifier);
            }
            traverse_cast(node->factor.num, symtab);
            traverse_cast(node->factor.expr, symtab);
            break;
        default:
            break;
    }

out:
            return global;
}

symbol_table_t *analyze_semantics(cast_node_t *cast_root)
{
    return traverse_cast(cast_root, NULL);
}
