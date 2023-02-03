#include "semantic_analyzer.h"
#include "../logging/logging.h"
#include "../config/globals.h"
#include "../io/io.h"
#include "../config/table_initializers.h"
#include "../expression_evaluator/expression_evaluator.h"
#include "../symbol_table/string_repository/string_symbol.h"

#include <stdlib.h>
#include <string.h>

SemanticAnalyzer *init_semantic_analyzer(AstNode *root, Lexer *lexer) {
    SemanticAnalyzer *analyzer = malloc(sizeof(SemanticAnalyzer));
    if (!analyzer)
        throw_memory_allocation_error(SEMANTIC_ANALYZER);

    analyzer->table = init_symbol_table();
    analyzer->scope_stack = init_scope_stack();
    analyzer->root = root;
    analyzer->root_func_name = DEFAULT_ROOT_FUNCTION_NAME;
    analyzer->error_count = 0;
    analyzer->lexer = lexer;

    return analyzer;
}

void semantic_analyzer_dispose(SemanticAnalyzer *analyzer) {
    symbol_table_dispose(analyzer->table);
    scope_stack_dispose(analyzer->scope_stack);
    free(analyzer);
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
char *validate_assignment(SemanticAnalyzer *analyzer, DataType type_dst, AstNode *value_node) {
    char *msg;
    DataType value_type = value_node->data.expression.value->type;
    if (value_node->type == AST_FUNCTION_CALL) {
        /* Function Call */
        // function calls are not supported right now
    } else {
        /* Assignment */
        // incompatible types
        if (!compare_types(type_dst, value_type)) {
            return alsprintf(&msg, "Type mismatch: Can't assign value from type %s to %s variable",
                             data_type_to_str(value_type), data_type_to_str(type_dst));
        }
        // invalid value for boolean
        if (type_dst == TYPE_BOOL &&
            value_node->data.expression.value->value.double_value != 0 &&
            value_node->data.expression.value->value.double_value != 1) {
            return strdup("Illegal value to a boolean variable. The allowed values are true (1) or false (0)");
        }
        if (value_type == TYPE_STRING) {
            // add string literals to the strings table
            string_repository_add_string_identifier(analyzer->table->str_repo,
                                                    strdup(value_node->data.expression.value->value.string_value));
        }
    }
    return NULL; // OK
}

/* This assumes that the root node is a compound
 * returns the number of errors found
 * */
int semantic_analyze_tree(SemanticAnalyzer *analyzer) {
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
        log_debug(SEMANTIC_ANALYZER, "Starting point missing, \"%s\" is not defined", analyzer->root_func_name);
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
    AstNode *curr_node;
    char *id_found;
    // add new scope
    scope_stack_push_scope(analyzer->scope_stack);
    // add all functions to scope
    for (i = 0; i < block->size; i++) {
        curr_node = (AstNode *) block->items[i];
        if (curr_node->type == AST_FUNCTION_DEFINITION) {
            id_found = scope_stack_lookup(analyzer->scope_stack, curr_node->data.function_definition.func_name);
            if (id_found) {
                log_debug(SEMANTIC_ANALYZER, "Function '%s' already defined",
                          curr_node->data.function_definition.func_name);
                analyzer->error_count += 1;
            } else {
                symbol_table_insert(analyzer->table,
                                    FUNCTION,
                                    curr_node->data.function_definition.func_name,
                                    (SymbolValue) {.func_symbol = (FunctionSymbol) {
                                            .func_name = curr_node->data.function_definition.func_name,
                                            .arg_types = curr_node->data.function_definition.args,
                                            .returned = 0
                                    }},
                                    curr_node);
                scope_stack_add_identifier(analyzer->scope_stack,
                                           strdup(curr_node->data.function_definition.func_name));
            }
        }
    }
    // analyze all the statements in the block
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
    }

    analyzer_func = hash_table_lookup(ast_type_to_analyzer_map, alsprintf(&id_ptr, "%d", node->type));
    free(id_ptr);
    if (analyzer_func) {
        analyzer_func(analyzer, node, parent);
    }
}

/* analyzes expression for variables that are declared before usage
 * if target_type is not -1, it checks that the expression does not contain elements from few types,
 * (for example, if the target var is int, an expression containing string is not allowed).
 * if target_type is -1, this check is ignored.
 * */
void semantic_analyze_expression(SemanticAnalyzer *analyzer, Expression *expr, DataType target_type) {
    int i;
    ArithmeticToken *curr_tok;
    for (i = 0; i < expr->tokens->size; i++) {
        curr_tok = expr->tokens->items[i];
        // check for an value that is not defined
        if (curr_tok->type == ID && scope_stack_lookup(analyzer->scope_stack, curr_tok->value.var) == NULL) {
            message_with_trace(SEMANTIC_ANALYZER, analyzer->lexer, curr_tok->original_tok->line,
                               curr_tok->original_tok->column,
                               curr_tok->original_tok->length,
                               "Target variable '%s' is not defined in the current scope. Try declaring it before usage: <variable_type> %s;",
                               curr_tok->value, curr_tok->value);
            analyzer->error_count += 1;
        }
        // if element is string and target var is not string, or the opposite
        if (curr_tok->type == STRING) {
            // add string literals to the strings table
            string_repository_add_string_identifier(analyzer->table->str_repo,
                                                    strdup(curr_tok->original_tok->value));

            if (target_type != -1 && target_type != STRING) {
                message_with_trace(SEMANTIC_ANALYZER, analyzer->lexer, curr_tok->original_tok->line,
                                   curr_tok->original_tok->column,
                                   curr_tok->original_tok->length,
                                   "String value cannot be assigned to %s variable",
                                   data_type_to_str(target_type));
                analyzer->error_count += 1;
            }
        } else if (target_type == STRING && curr_tok->type != STRING) {
            message_with_trace(SEMANTIC_ANALYZER, analyzer->lexer, curr_tok->original_tok->line,
                               curr_tok->original_tok->column,
                               curr_tok->original_tok->length,
                               "%s cannot be assigned to a string variable",
                               data_type_to_str(target_type), data_type_to_str(target_type));
            analyzer->error_count += 1;
        }
    }
}

void semantic_analyze_variable_declaration(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *err_msg = NULL, *id_found;

    // check if a variable with the same name exists in the current or ancestor scopes
    id_found = scope_stack_lookup(analyzer->scope_stack, node->data.variable_declaration.var->name);
    if (id_found) { // if value exists
        log_debug(SEMANTIC_ANALYZER, "Variable '%s' already defined", node->data.variable_declaration.var->name);
        analyzer->error_count += 1;
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

    if (node->data.variable_declaration.value->data.expression.contains_variables) {
        semantic_analyze_expression(analyzer, &node->data.variable_declaration.value->data.expression,
                                    node->data.variable_declaration.var->value->type);
    } else {
        // check that the initial value that is has been assigned to the variable is of the same type
        err_msg = validate_assignment(analyzer, node->data.variable_declaration.var->value->type,
                                      node->data.variable_declaration.value);
    }
    if (err_msg) {
        // assignment invalid
        log_debug(SEMANTIC_ANALYZER, err_msg);
        analyzer->error_count += 1;
    }
}

void semantic_analyze_assignment(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    char *err_msg = NULL, *id_found;
    Symbol *target_var;

    target_var = symbol_table_lookup(analyzer->table, node->data.assignment.dst_variable->value);
    id_found = scope_stack_lookup(analyzer->scope_stack, node->data.assignment.dst_variable->value);
    // if the target var is undefined
    if (!id_found) {
        log_debug(SEMANTIC_ANALYZER,
                  "Target variable '%s' is not defined in the current scope. Try declaring it before usage: <variable_type> %s;",
                  node->data.assignment.dst_variable->value, node->data.assignment.dst_variable->value);
        analyzer->error_count += 1;
    }
    // if the target var is not a variable (it is not allowed to assign values to a function...)
    if (target_var->type != VARIABLE) {
        log_debug(SEMANTIC_ANALYZER, "Assigning values to a function is not allowed, only to variables");
        analyzer->error_count += 1;
    }

    if (node->data.assignment.expression->data.expression.contains_variables) {
        semantic_analyze_expression(analyzer, &node->data.assignment.expression->data.expression,
                                    target_var->value.var_symbol.type);
    } else {
        err_msg = validate_assignment(analyzer, target_var->value.var_symbol.type, node->data.assignment.expression);
    }
    if (err_msg) {
        // assignment invalid
        log_debug(SEMANTIC_ANALYZER, err_msg);
        analyzer->error_count += 1;
    }
}

void semantic_analyze_function(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    // analyze function body
    semantic_analyze_block(analyzer, node->data.function_definition.body, node);
    // check if return statement was met, and the function supposed to return something
    if (node->data.function_definition.returnType != TYPE_VOID &&
        !(symbol_table_lookup(analyzer->table, node->data.function_definition.func_name)->value.func_symbol.returned)) {
        log_debug(SEMANTIC_ANALYZER, "Function %s does not return anything", node->data.function_definition.func_name);
        analyzer->error_count += 1;
    }
}

void semantic_analyze_start_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    analyzer->root_func_name = node->data.start_expr.starting_point;
}

void semantic_analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    DataType condition_type = node->data.if_statement.condition->data.expression.value->type;
    if (condition_type != TYPE_INT && condition_type != TYPE_DOUBLE) {
        log_debug(SEMANTIC_ANALYZER, "Condition in if statement should have a boolean value, not %s",
                  data_type_to_str(condition_type));
        analyzer->error_count += 1;
    }
    // analyze if body
    semantic_analyze_block(analyzer, node->data.if_statement.body_node, parent);
    // analyze else
    semantic_analyze_block(analyzer, node->data.if_statement.else_node, parent);
}

void semantic_analyze_loop_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    Expression *start, *end;
    start = node->data.loop.start;
    end = node->data.loop.end;
    if (!start->contains_variables && !end->contains_variables) {
        if (start->value->value.double_value == 0 && (int) end->value->value.double_value <= 0) {
            log_debug(SEMANTIC_ANALYZER, "Loop amount must be a natural number.");
            analyzer->error_count += 1;
        }
        // if loop goes backwards
        if (node->data.loop.loop_counter_name && end->value->value.double_value < start->value->value.double_value) {
            node->data.loop.forward = 0;
        }
    } else {
        semantic_analyze_expression(analyzer, start, -1);
        semantic_analyze_expression(analyzer, end, -1);
    }
    // if there is a loop counter
    if (node->data.loop.loop_counter_name != NULL) {
        Token *err_tok = ((ArithmeticToken *) end->tokens->items[0])->original_tok;
        if (scope_stack_lookup(analyzer->scope_stack, node->data.loop.loop_counter_name) != NULL
            && symbol_table_lookup(analyzer->table, node->data.loop.loop_counter_name)->value.var_symbol.type !=
               TYPE_INT) {
            message_with_trace(SEMANTIC_ANALYZER, analyzer->lexer, err_tok->line,
                               node->data.loop.loop_counter_col, strlen(node->data.loop.loop_counter_name),
                               "Only an integer variable can be used as a loop counter");
            analyzer->error_count += 1;
        }

        scope_stack_push_scope(analyzer->scope_stack);
        scope_stack_add_identifier(analyzer->scope_stack, strdup(node->data.loop.loop_counter_name));
        symbol_table_insert(analyzer->table,
                            VARIABLE,
                            node->data.loop.loop_counter_name,
                            (SymbolValue) {.var_symbol = (VariableSymbol) {
                                    .var_name = node->data.loop.loop_counter_name,
                                    .type = TYPE_INT
                            }},
                            node);
    }
    // analyze body
    semantic_analyze_block(analyzer, node->data.loop.body, parent);
    if (node->data.loop.loop_counter_name != NULL) {
        // pop the scope containing the loop counter
        scope_stack_pop_scope(analyzer->scope_stack);
    }
}

void semantic_analyze_while_loop(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    semantic_analyze_expression(analyzer, &node->data.while_loop.condition->data.expression, TYPE_BOOL);
    // analyze body
    semantic_analyze_block(analyzer, node->data.while_loop.body, parent);
}

// parent should be function definition
void semantic_analyze_return_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    DataType parent_return_type = parent->data.function_definition.returnType;
    DataType return_type = node->data.return_statement.value_expr->data.expression.value->type;
    double double_return_val = node->data.return_statement.value_expr->data.expression.value->value.double_value;
    // void function does not supposed to return anything
    if (parent_return_type == TYPE_VOID && return_type != TYPE_VOID) {
        log_debug(SEMANTIC_ANALYZER, "Function %s does not supposed to return anything",
                  parent->data.function_definition.func_name);
        analyzer->error_count += 1;
    }
    // if return types are different, and both of them are not int, double or char, which can be cast from each other
//    if (!compare_types(return_type, parent_return_type)) {
    if (validate_assignment(analyzer, parent_return_type, node->data.return_statement.value_expr) != NULL) {
        log_debug(SEMANTIC_ANALYZER, "Type mismatch: Returning %s from a function that supposed to return %s",
                  data_type_to_str(return_type), data_type_to_str(parent_return_type));
        analyzer->error_count += 1;
    }
    if (parent_return_type == TYPE_BOOL && double_return_val != 0 && double_return_val != 1) {
        log_debug(SEMANTIC_ANALYZER, "Value %.3f is not boolean, as expected to be returned from %s",
                  double_return_val, parent->data.function_definition.func_name);
        analyzer->error_count += 1;
    }
    // mark that the function has reached a return statement
    symbol_table_lookup(analyzer->table, parent->data.function_definition.func_name)->value.func_symbol.returned = 1;
    // store the number of arguments of the parent function
    node->data.return_statement.parent_function_arg_count = parent->data.function_definition.args->size;
}

void semantic_analyze_function_call(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent) {
    int i;
    Symbol *target_func = symbol_table_lookup(analyzer->table, node->data.function_call.func_name);

    // check if function exists
    if (target_func == NULL) {
        log_debug(SEMANTIC_ANALYZER, "Function %s does not exist in the current context",
                  node->data.function_call.func_name);
        analyzer->error_count += 1;
        return; // cant analyze a function that does not exist
    }
    if (target_func->type != FUNCTION) {
        log_debug(SEMANTIC_ANALYZER, "This identifier is not callable: %s",
                  node->data.function_call.func_name);
        analyzer->error_count += 1;
    }

    // check that the count of the arguments is matching with the target function
    if (node->data.function_call.args->size != target_func->value.func_symbol.arg_types->size) {
        log_debug(SEMANTIC_ANALYZER, "Expecting %d argument%s to pass to %s, got %d.",
                  target_func->value.func_symbol.arg_types->size,
                  target_func->value.func_symbol.arg_types->size == 1 ? "" : "s",
                  target_func->value.func_symbol.func_name,
                  node->data.function_call.args->size);
        analyzer->error_count += 1;
        return; // cant check the arguments if not the correct amount is passed
    }

    // check every argument
    for (i = 0; i < node->data.function_call.args->size; i++) {
        semantic_analyze_expression(analyzer, &((AstNode *) node->data.function_call.args->items[i])->data.expression,
                                    ((Variable *) target_func->value.func_symbol.arg_types->items[i])->value->type);
    }
}
