#ifndef INFINITY_COMPILER_SEMANTIC_ANALYZER_H
#define INFINITY_COMPILER_SEMANTIC_ANALYZER_H

#include "../symbol_table/symbol_table.h"

typedef struct {
    SymbolTable *table;
    AstNode *root; // the root of the AST tree, given by the parser. the start of the analysis starts here
    char *root_func_name; // the name of the function that acts as the starting point
    AstNode *starting_point; // the starting point of the application. will be used by the code generator
} SemanticAnalyzer;

SemanticAnalyzer *init_semantic_analyzer(AstNode *root);

void semantic_analyze_tree(SemanticAnalyzer *analyzer);

int compare_types(DataType type_a, DataType type_b);

void semantic_analyze_start_statement(SemanticAnalyzer *analyzer, AstNode *node);

void semantic_analyze_function(SemanticAnalyzer *analyzer, AstNode *node);

void semantic_analyze_block(SemanticAnalyzer *analyzer, List *block, AstNode *parent);

void semantic_analyze_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_variable_declaration(SemanticAnalyzer *analyzer, AstNode *node);

void semantic_analyze_assignment(SemanticAnalyzer *analyzer, AstNode *node);

void semantic_analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_loop_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyze_return_statement(SemanticAnalyzer *analyzer, AstNode *node, AstNode *parent);

void semantic_analyzer_dispose(SemanticAnalyzer *analyzer);

#endif //INFINITY_COMPILER_SEMANTIC_ANALYZER_H
