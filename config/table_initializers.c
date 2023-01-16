#include "table_initializers.h"
#include "../ast/ast.h"
#include "../semantic_analyzer/semantic_analyzer.h"
#include "../io/io.h"
#include "../parser/parser.h"
#include <stdlib.h>

HashTable *ast_type_to_analyzer_table;

void init_ast_type_to_analyzer_table() {
    int i;
    char *id_ptr;
    AstType types[] = {
            AST_START_EXPRESSION,
            AST_VARIABLE_DECLARATION,
            AST_ASSIGNMENT,
            AST_FUNCTION_DEFINITION,
//            AST_FUNCTION_CALL,
            AST_IF_STATEMENT,
            AST_LOOP,
            AST_RETURN_STATEMENT,
    };
    void (*funcs[])(SemanticAnalyzer *, AstNode *, AstNode *) = {
            semantic_analyze_start_statement,
            semantic_analyze_variable_declaration,
            semantic_analyze_assignment,
            semantic_analyze_function,
//            NULL,
            semantic_analyze_if_statement,
            semantic_analyze_loop_statement,
            semantic_analyze_return_statement,
    };
    ast_type_to_analyzer_table = init_hash_table(19, NULL); // no need to free the functions...

    for (i = 0; i < ARRLEN(types); i++) {
        hash_table_insert(ast_type_to_analyzer_table, alsprintf(&id_ptr, "%d", types[i]), funcs[i]);
    }
}

HashTable *statement_to_parser_table;

void init_statement_to_parser_table() {
    int i;
    char *id_ptr;
    TokenType types[] = {
            START_KEYWORD,
            ID,
            FUNC_KEYWORD,
            INT_KEYWORD,
            CHAR_KEYWORD,
            BOOL_KEYWORD,
            STRING_KEYWORD,
            IF_KEYWORD,
            LOOP_KEYWORD,
            RETURN_KEYWORD,
    };
    AstNode *(*funcs[])(Parser *) = {
            parser_parse_start_expression,
            parser_parse_id,
            parser_parse_function_definition,
            parser_parse_var_declaration,
            parser_parse_var_declaration,
            parser_parse_var_declaration,
            parser_parse_var_declaration,
            parser_parse_if_statement,
            parser_parse_loop,
            parser_parse_return_statement,
    };
    statement_to_parser_table = init_hash_table(19, NULL); // no need to free the functions...

    for (i = 0; i < ARRLEN(types); i++) {
        hash_table_insert(statement_to_parser_table, alsprintf(&id_ptr, "%d", types[i]), funcs[i]);
    }
}
