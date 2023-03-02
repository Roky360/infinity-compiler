#ifndef INFINITY_COMPILER_SEMANTIC_ANALYZER_H
#define INFINITY_COMPILER_SEMANTIC_ANALYZER_H

#include "../symbol_table/symbol_table.h"
#include "../ast/ast.h"
#include "../scope_stack/scope_stack.h"
#include "../lexer/lexer.h"

/** Built-in Functions */
#define PRINT_FUNC "print"
#define PRINTLN_FUNC "println"
#define EXIT_FUNC "exit"

typedef struct {
    SymbolTable *table;
    ScopeStack *scope_stack;
    AstNode *root; // the root of the AST tree, given by the parser. the analysis starts here
    char *root_func_name; // the name of the function that acts as the starting point
    AstNode *starting_point; // the starting point of the application. will be used by the code generator
    int error_count; // counts the number of semantic errors found

    Lexer *lexer; // for error printing
} SemanticAnalyzer;

SemanticAnalyzer *init_semantic_analyzer(AstNode *root, Lexer *lexer);

void semantic_analyzer_dispose(SemanticAnalyzer *analyzer);

/// Checks if two data types can be assigned to each other.
/// \param type_a
/// \param type_b
/// \return true if the type_b can be casted to type_a
int compare_types(DataType type_a, DataType type_b);

/// Checks for a valid assignment.
/// - For function calls - not supported
/// - For statements - checks that the types are matching, or can be cast, via the compare_types method.
/// Then, if the target is of type bool, checks that the expression value_node is 0 or 1.
/// Every error message is allocated, so it has to be freed after usage.
/// \param analyzer
/// \param type_dst Destination variable type
/// \param value_node The node containing the expression that will be assigned.
/// \return The error message according to the error found. If the assignment is valid, returns NULL.
char *validate_assignment(SemanticAnalyzer *analyzer, DataType type_dst, AstNode *value_node);

void semantic_add_builtin_functions_to_scope(SemanticAnalyzer *analyzer);

/// Main function to analyze an AST tree.
/// \param analyzer
/// \return The number of semantic errors found.
int semantic_analyze_tree(SemanticAnalyzer *analyzer);

/// Analyzes a block of code (between curly braces)
/// \param analyzer
/// \param block List of expressions inside the block
/// \param parent Parent node of the code block.
void semantic_analyze_block(SemanticAnalyzer *analyzer, List *block, AstNode *parent);

/// Distributes a node to its analyzer function, according to its type.
/// Uses the ast_type_to_analyzer_map hash table for O(1) time complexity.
/// \param analyzer
/// \param node
/// \param parent Parent node of `node`;
void semantic_analyze_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

/// Analyzes an expression
/// \param analyzer
/// \param expr
/// \param target_type
void semantic_analyze_expression(SemanticAnalyzer *analyzer, Expression *expr, DataType target_type);

/// Analyses a `variable declaration` expression
/// \param analyzer
/// \param node
/// \param parent
void semantic_analyze_variable_declaration(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

/// Analyses an `assignment` expression
/// \param analyzer
/// \param node
/// \param parent
void semantic_analyze_assignment(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_function(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_start_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_loop_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_while_loop(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_return_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_function_call(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_swap_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

#endif //INFINITY_COMPILER_SEMANTIC_ANALYZER_H
