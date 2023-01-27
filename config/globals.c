#include "globals.h"
#include "table_initializers.h"

TokenType data_types[] = {VOID_KEYWORD, INT_KEYWORD, BOOL_KEYWORD, CHAR_KEYWORD, STRING_KEYWORD};
int data_types_len = ARRLEN(data_types);

void init_globals() {
    init_ast_type_to_analyzer_table();
    init_statement_to_parser_table();
    // lexer tables
    init_char_to_lexing_function_map();
    init_char_to_token_type_map();
    init_id_to_keyword_map();
    // expression evaluator
    init_operator_to_applier_function_map();
    init_precedence_map();
    // code generator
    init_statement_to_generator_map();
    init_operator_to_generator_map();
}

void clean_globals() {
    // free the helper forwarding-tables
    hash_table_dispose(ast_type_to_analyzer_map);
    hash_table_dispose(statement_to_parser_map);
    // lexer tables
    hash_table_dispose(char_to_to_lexing_function_map);
    hash_table_dispose(char_to_token_type_map);
    hash_table_dispose(id_to_keyword_map);
    // expression evaluator
    hash_table_dispose(operator_to_applier_function_map);
    hash_table_dispose(precedence_map);
    // code generator
    hash_table_dispose(statement_to_generator_map);
    hash_table_dispose(operator_to_generator_map);
}
