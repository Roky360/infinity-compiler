#ifndef INFINITY_COMPILER_SEMANTIC_ANALYZER_H
#define INFINITY_COMPILER_SEMANTIC_ANALYZER_H

#include "../symbol_table/symbol_table.h"
#include "../ast/ast.h"
#include "../scope_stack/scope_stack.h"

typedef struct {
    SymbolTable *table;
    ScopeStack *scope_stack;
    AstNode *root; // the root of the AST tree, given by the parser. the start of the analysis starts here
    char *root_func_name; // the name of the function that acts as the starting point
    AstNode *starting_point; // the starting point of the application. will be used by the code generator
    int error_count; // counts the number of semantic errors found
} SemanticAnalyzer;

SemanticAnalyzer *init_semantic_analyzer(AstNode *root);

int compare_types(DataType type_a, DataType type_b);

char *validate_assignment(DataType type_dst, AstNode *value_node);

int semantic_analyze_tree(SemanticAnalyzer *analyzer);

void semantic_analyze_block(SemanticAnalyzer *analyzer, List *block, AstNode *parent);

void semantic_analyze_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_variable_declaration(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_assignment(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_function(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_start_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_loop_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_return_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyzer_dispose(SemanticAnalyzer *analyzer);

#endif //INFINITY_COMPILER_SEMANTIC_ANALYZER_H
