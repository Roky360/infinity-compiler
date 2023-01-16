#include "globals.h"
#include "table_initializers.h"

TokenType data_types[] = {VOID_KEYWORD, INT_KEYWORD, BOOL_KEYWORD, CHAR_KEYWORD, STRING_KEYWORD};
int data_types_len = ARRLEN(data_types);

void init_globals() {
    init_ast_type_to_analyzer_table();
    init_statement_to_parser_table();
}

void clean_globals() {
    // free the helper forwarding-tables
    hash_table_dispose(ast_type_to_analyzer_table);
    hash_table_dispose(statement_to_parser_table);
}
