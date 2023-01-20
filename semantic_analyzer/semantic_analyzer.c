#include "semantic_analyzer.h"
#include "../symbol_table/symbol/symbol.h"
#include "../logging/logging.h"
#include "../config/globals.h"
#include "../io/io.h"
#include "../config/table_initializers.h"

#include <stdlib.h>
#include <string.h>

SemanticAnalyzer *init_semantic_analyzer(AstNode *root) {
    SemanticAnalyzer *analyzer = malloc(sizeof(SemanticAnalyzer));
    if (!analyzer)
        throw_memory_allocation_error(SEMANTIC_ANALYZER);

    analyzer->table = init_symbol_table();
    analyzer->scope_stack = init_scope_stack();
    analyzer->root = root;
    analyzer->root_func_name = DEFAULT_ROOT_FUNCTION_NAME;
    analyzer->error_count = 0;

    return analyzer;
}

/**
 * returns true if the type_b can be casted to type_a
 * */
int compare_types(DataType type_a, DataType type_b) {
    return type_a == type_b ||
           ((type_b == TYPE_INT || type_b == TYPE_DOUBLE || type_b == TYPE_CHAR || type_b == TYPE_BOOL) &&
            (type_a == TYPE_INT || type_a == TYPE_DOUBLE || type_a == TYPE_CHAR || type_a == TYPE_BOOL));
}

/* Checks for a valid assignment.
 * - For function calls - not supported
 * - For statements - checks that the types are matching, or can be cast, via the compare_types method.
 * Then, if the target is of type bool, checks that the expression value_node is 0 or 1.
 * Returns the error message according to the error found. If the assignment is valid, returns NULL.
 * Every error message is allocated, so it has to be freed after usage.
 * */
char *validate_assignment(DataType type_dst, AstNode *value_node) {
    char *msg;
    DataType value_type = value_node->data.expression.value->type;
    if (value_node->type == AST_FUNCTION_CALL) {
        /* Function Call */
        // function calls are not supported right now
    } else {
        /* Assignment */
        // incompatible types
        if (!compare_types(type_dst, value_type)) {
            alsprintf(&msg, "Type mismatch: Can't assign value_node from type %s to %s variable",
                      data_type_to_str(value_type), data_type_to_str(type_dst));
            return msg;
        }
        // invalid value for boolean
        if (type_dst == TYPE_BOOL &&
            value_node->data.expression.value->value.double_value != 0 &&
            value_node->data.expression.value->value.double_value != 1) {
            return strdup("Illegal value_node to a boolean variable. The allowed values are true (1) or false (0)");
        }
    }
    return NULL; // OK
}

/* This assumes that the root node is a compound
 * returns the number of errors found
 * */
int semantic_analyze_tree(SemanticAnalyzer *analyzer) {
    char *errMsg;
    Symbol *starting_point_entry;

    if (analyzer->root->type != AST_COMPOUND) {
        fprintf(stderr, "Expecting a compound node as the root node of the application\n");
        exit(1);
    }

    // analyze the tree
    semantic_analyze_block(analyzer, analyzer->root->data.compound.children, analyzer->root);

    // check that the starting point function exists
    starting_point_entry = symbol_table_lookup(analyzer->table, analyzer->root_func_name);
    if (!starting_point_entry) {
        alsprintf(&errMsg, "Starting point missing, \"%s\" is not defined", analyzer->root_func_name);
        log_debug(SEMANTIC_ANALYZER, errMsg);
        analyzer->error_count += 1;
    } else {
        analyzer->starting_point = starting_point_entry->initializer;
    }

    return analyzer->error_count;
}

/** Analyzes a block.
 * Adds a new scope to the scope stack, and pops it at the end
 * */
void semantic_analyze_block(SemanticAnalyzer *analyzer, List *block, AstNode *parent) {
    int i;
    // add new scope
    scope_stack_push_scope(analyzer->scope_stack);
    for (i = 0; i < block->size; i++) {
        semantic_analyze_statement(analyzer, (AstNode *) block->items[i], parent);
    }
    // pop the scope
    scope_stack_pop_scope(analyzer->scope_stack);
}

void semantic_analyze_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *id_ptr;
    void (*analyzer_func)(SemanticAnalyzer *, AstNode *, AstNode *);

    // alert for unreachable code
    if (parent->type == AST_FUNCTION_DEFINITION &&
        symbol_table_lookup(analyzer->table, parent->data.function_definition.func_name)->value.func_symbol.returned) {
        // TODO: link this warning to the better logging system
        log_debug(SEMANTIC_ANALYZER, "Unreachable code");
        printf("%d\n", node->type);
    }

    analyzer_func = hash_table_lookup(ast_type_to_analyzer_map, alsprintf(&id_ptr, "%d", node->type));
    free(id_ptr);
    if (analyzer_func) {
        analyzer_func(analyzer, node, parent);
    }
}

void semantic_analyze_variable_declaration(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *err_msg, *id_found;

    // check if a variable with the same name exists in the current or ancestor scopes
    id_found = scope_stack_lookup(analyzer->scope_stack, node->data.variable_declaration.var->name);
    if (id_found) { // if id exists
        alsprintf(&err_msg, "Variable '%s' already defined", node->data.variable_declaration.var->name);
        log_error(SEMANTIC_ANALYZER, err_msg);
    } else {
        symbol_table_insert(analyzer->table,
                            VARIABLE,
                            node->data.variable_declaration.var->name,
                            (SymbolValue) {.var_symbol = (VariableSymbol) {
                                    .var_name = node->data.variable_declaration.var->name,
                                    .type = node->data.variable_declaration.var->value->type,
                            }},
                            node);
        scope_stack_add_identifier(analyzer->scope_stack, strdup(node->data.variable_declaration.var->name));
    }

    // check that the initial value that is has been assigned to the variable is of the same type
    err_msg = validate_assignment(node->data.variable_declaration.var->value->type,
                                  node->data.variable_declaration.value);
    if (err_msg) {
        // assignment invalid
        log_error(SEMANTIC_ANALYZER, err_msg);
    }
}

// TODO: stopped here. continue adding scope
void semantic_analyze_assignment(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *err_msg, *id_found;
    Symbol *target_var;

    target_var = symbol_table_lookup(analyzer->table, node->data.assignment.dst_variable->value);
    id_found = scope_stack_lookup(analyzer->scope_stack, node->data.assignment.dst_variable->value);
    // if the target var is undefined
    if (!id_found) {
        alsprintf(&err_msg,
                  "Target variable '%s' is not defined in the current scope. Try declaring it before usage: <variable_type> %s;",
                  node->data.assignment.dst_variable->value, node->data.assignment.dst_variable->value);
        log_error(SEMANTIC_ANALYZER, err_msg);
    }
    // if the target var is not a variable (it is not allowed to assign values to a function...)
    if (target_var->type != VARIABLE) {
        alsprintf(&err_msg, "Assigning values to a function is not allowed, only to variables");
        log_error(SEMANTIC_ANALYZER, err_msg);
    }

    validate_assignment(target_var->value.var_symbol.type, node->data.assignment.expression);
}

void semantic_analyze_function(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *errMsg, *id_found;
    id_found = scope_stack_lookup(analyzer->scope_stack, node->data.function_definition.func_name);
    if (id_found) {
        alsprintf(&errMsg, "Function '%s' already defined", node->data.function_definition.func_name);
        log_error(SEMANTIC_ANALYZER, errMsg);
    } else {
        symbol_table_insert(analyzer->table,
                            FUNCTION,
                            node->data.function_definition.func_name,
                            (SymbolValue) {.func_symbol = (FunctionSymbol) {
                                    .func_name = node->data.function_definition.func_name,
                                    .arg_types = node->data.function_definition.args,
                                    .returned = 0
                            }},
                            node);
        scope_stack_add_identifier(analyzer->scope_stack, strdup(node->data.function_definition.func_name));
    }

    // analyze function body
    semantic_analyze_block(analyzer, node->data.function_definition.body, node);
    // check if return statement was met, and the function supposed to return something
    if (node->data.function_definition.returnType != TYPE_VOID &&
        !(symbol_table_lookup(analyzer->table, node->data.function_definition.func_name)->value.func_symbol.returned)) {
        alsprintf(&errMsg, "Function %s does not return anything", node->data.function_definition.func_name);
        log_error(SEMANTIC_ANALYZER, errMsg);
    }
}

void semantic_analyze_start_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    analyzer->root_func_name = node->data.start_expr.starting_point;
}

void semantic_analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *err_msg;
    DataType condition_type = node->data.if_statement.condition->value->type;
    if (condition_type != TYPE_INT) {
        alsprintf(&err_msg, "Condition in if statement should have a boolean value, not %s",
                  data_type_to_str(condition_type));
    }
    // analyze if body
    semantic_analyze_block(analyzer, node->data.if_statement.body_node, parent);
    // analyze else
    semantic_analyze_block(analyzer, node->data.if_statement.else_node, parent);
}

void semantic_analyze_loop_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    int start, end;
    start = node->data.loop.start;
    end = node->data.loop.end;
    if (start == 0 && end < 0) {
        log_error(SEMANTIC_ANALYZER, "Loop amount must be a natural number.");
    }
    // if loop goes backwards
    if (node->data.loop.loop_counter_name && end > start) {
        node->data.loop.forward = 0;
    }
    // analyze body
    semantic_analyze_block(analyzer, node->data.loop.body, parent);
}

// parent should be function definition
void semantic_analyze_return_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *errMsg;
    DataType parent_return_type = parent->data.function_definition.returnType;
    DataType return_type = node->data.return_statement.value_expr->data.expression.value->type;
    // void function does not supposed to return anything
    if (parent_return_type == TYPE_VOID && return_type != TYPE_VOID) {
        alsprintf(&errMsg, "Function %s does not supposed to return anything",
                  parent->data.function_definition.func_name);
        log_error(SEMANTIC_ANALYZER, errMsg);
    }
    // if return types are different, and both of them are not int, double or char, which can be cast from each other
    if (!compare_types(return_type, parent_return_type)) {
        alsprintf(&errMsg, "Type mismatch: Returning %s from a function that supposed to return %s",
                  data_type_to_str(return_type), data_type_to_str(parent_return_type));
        log_error(SEMANTIC_ANALYZER, errMsg);
    }
    // mark that the function has reached a return statement
    symbol_table_lookup(analyzer->table, parent->data.function_definition.func_name)->value.func_symbol.returned = 1;
}

void semantic_analyzer_dispose(SemanticAnalyzer *analyzer) {
    symbol_table_dispose(analyzer->table);
    scope_stack_dispose(analyzer->scope_stack);
    free(analyzer);
}
