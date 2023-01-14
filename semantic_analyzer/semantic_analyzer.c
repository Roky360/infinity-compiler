#include "semantic_analyzer.h"
#include "../logging/logging.h"
#include "../config/globals.h"
#include "../io/io.h"

#include <stdlib.h>

SemanticAnalyzer *init_semantic_analyzer(AstNode *root) {
    SemanticAnalyzer *analyzer = malloc(sizeof(SemanticAnalyzer));
    if (!analyzer)
        throw_memory_allocation_error(SEMANTIC_ANALYZER);

    analyzer->table = init_symbol_table();
    analyzer->root = root;
    analyzer->root_func_name = DEFAULT_ROOT_FUNCTION_NAME;

    return analyzer;
}

// This assumes that the root node is a compound
void semantic_analyze_tree(SemanticAnalyzer *analyzer) {
    char *errMsg;
    SymbolEntry *starting_point_entry;

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
        log_error(SEMANTIC_ANALYZER, errMsg);
    }
    analyzer->starting_point = starting_point_entry->initializer;
}

/**
 * returns true if the type_b can be casted to type_a
 * */
int compare_types(DataType type_a, DataType type_b) {
    return type_a == type_b ||
           ((type_b == TYPE_INT || type_b == TYPE_DOUBLE || type_b == TYPE_CHAR || type_b == TYPE_BOOL) &&
            (type_a == TYPE_INT || type_a == TYPE_DOUBLE || type_a == TYPE_CHAR || type_a == TYPE_BOOL));
}

void semantic_analyze_start_statement(SemanticAnalyzer *analyzer, AstNode *node) {
    analyzer->root_func_name = node->data.start_expr.starting_point;
}

void semantic_analyze_function(SemanticAnalyzer *analyzer, AstNode *node) {
    char *errMsg;
    int insertion_successful;
    insertion_successful = symbol_table_insert(analyzer->table,
                                               FUNCTION,
                                               node->data.function_definition.func_name,
                                               (SymbolValue) {.func_symbol = (FunctionSymbol) {
                                                       .func_name = node->data.function_definition.func_name,
                                                       .arg_types = node->data.function_definition.args,
                                                       .returned = 0
                                               }},
                                               node);
    if (!insertion_successful) {
        alsprintf(&errMsg, "Function '%s' already defined", node->data.function_definition.func_name);
        log_error(SEMANTIC_ANALYZER, errMsg);
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

void semantic_analyze_variable_declaration(SemanticAnalyzer *analyzer, AstNode *node) {
    char *errMsg;
    int insertion_successful;
    DataType var_type, value_type;
    insertion_successful = symbol_table_insert(analyzer->table,
                                               VARIABLE,
                                               node->data.variable_declaration.var->name,
                                               (SymbolValue) {.var_symbol = (VariableSymbol) {
                                                       .var_name = node->data.variable_declaration.var->name,
                                                       .type = node->data.variable_declaration.var->value->type,
                                               }},
                                               node);
    if (!insertion_successful) {
        alsprintf(&errMsg, "Variable '%s' already defined", node->data.variable_declaration.var->name);
        log_error(SEMANTIC_ANALYZER, errMsg);
    }

    var_type = node->data.variable_declaration.var->value->type;
    if (node->data.variable_declaration.value->type == AST_FUNCTION_CALL) {
        // handle function call
        value_type = -1;
    } else {
        // handle expression
        value_type = node->data.variable_declaration.value->data.expression.value->type;
    }
    // type check
    if (var_type == TYPE_BOOL &&
        (node->data.variable_declaration.value->data.expression.value->value.double_value > 1 ||
         node->data.variable_declaration.value->data.expression.value->value.double_value < 0)) {
        log_error(SEMANTIC_ANALYZER,
                  "Illegal value to a boolean variable. The allowed values are true (1) or false (0)");
    } else if (!compare_types(var_type, value_type)) {
        alsprintf(&errMsg, "Type mismatch: Can't assign value from type %s to a %s variable",
                  data_type_to_str(value_type), data_type_to_str(var_type));
        log_error(SEMANTIC_ANALYZER, errMsg);
    }
}

void semantic_analyze_block(SemanticAnalyzer *analyzer, List *block, AstNode *parent) {
    int i;
    for (i = 0; i < block->size; i++) {
        semantic_analyze_statement(analyzer, (AstNode *) block->items[i], parent);
    }
}

void semantic_analyze_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    // alert for unreachable code
    if (parent->type == AST_FUNCTION_DEFINITION &&
        symbol_table_lookup(analyzer->table, parent->data.function_definition.func_name)->value.func_symbol.returned) {
        // TODO: link this warning to the better logging system
        log_debug(SEMANTIC_ANALYZER, "Unreachable code");
    }
    switch (node->type) {
        case AST_START_EXPRESSION:
            semantic_analyze_start_statement(analyzer, node);
            break;
        case AST_VARIABLE_DECLARATION:
            semantic_analyze_variable_declaration(analyzer, node);
            break;
        case AST_ASSIGNMENT:
            semantic_analyze_assignment(analyzer, node);
            break;
        case AST_FUNCTION_DEFINITION:
            semantic_analyze_function(analyzer, node);
            break;
        case AST_FUNCTION_CALL:
            // not supported yet
            break;
        case AST_IF_STATEMENT:
            semantic_analyze_if_statement(analyzer, node, parent);
            break;
        case AST_LOOP:
            semantic_analyze_loop_statement(analyzer, node, parent);
            break;
        case AST_RETURN_STATEMENT:
            semantic_analyze_return_statement(analyzer, node, parent);
            break;
        default:
            break;
    }
}

void semantic_analyze_assignment(SemanticAnalyzer *analyzer, AstNode *node) {
    char *errMsg;
    SymbolEntry *target_var;
    DataType src_type, dst_type;
    target_var = symbol_table_lookup(analyzer->table, node->data.assignment.dst_variable->value);
    // if the target var is undefined
    if (!target_var) {
        alsprintf(&errMsg, "Target variable '%s' is undefined. Try declaring it before usage: <variable_type> %s;",
                  node->data.assignment.dst_variable->value, node->data.assignment.dst_variable->value);
        log_error(SEMANTIC_ANALYZER, errMsg);
    }
    // if the target var is not a variable (it is not allowed to assign values to a function...)
    if (target_var->type != VARIABLE) {
        alsprintf(&errMsg, "Assigning values to a function is not allowed, only to variables");
        log_error(SEMANTIC_ANALYZER, errMsg);
    }

    src_type = node->data.assignment.expression->data.expression.value->type;
    dst_type = target_var->value.var_symbol.type;
    switch (node->data.assignment.expression->type) {
        case AST_EXPRESSION:
            if (!compare_types(src_type, dst_type)) {
                alsprintf(&errMsg, "Type mismatch: Can't assign value from type %s to a %s variable",
                          data_type_to_str(src_type), data_type_to_str(dst_type));
                log_error(SEMANTIC_ANALYZER, errMsg);
            }
            break;
        case AST_FUNCTION_CALL:
            // CHECK FUNCTION CALL
//            node->data.assignment.expression->data.function_call.func_name
            break;
        default:
            // TODO: better error handling with trace (here and anywhere else)
            alsprintf(&errMsg, "Expected an expression or a function call to assign to '%s'",
                      node->data.assignment.dst_variable->value);
            log_error(SEMANTIC_ANALYZER, errMsg);
            break;
    }
}

void semantic_analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *errMsg;
    DataType condition_type = node->data.if_statement.condition->value->type;
    if (condition_type != TYPE_INT) {
        alsprintf(&errMsg, "Condition in if statement should have a boolean value, not %s",
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
    // going backwards
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
    free(analyzer);
}
