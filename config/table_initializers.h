#ifndef INFINITY_COMPILER_TABLE_INITIALIZERS_H
#define INFINITY_COMPILER_TABLE_INITIALIZERS_H

#include "globals.h"

extern HashTable *ast_type_to_analyzer_table;
extern HashTable *statement_to_parser_table;

void init_ast_type_to_analyzer_table();

void init_statement_to_parser_table();

#endif //INFINITY_COMPILER_TABLE_INITIALIZERS_H
