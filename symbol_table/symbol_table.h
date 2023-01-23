#ifndef INFINITY_COMPILER_SYMBOL_TABLE_H
#define INFINITY_COMPILER_SYMBOL_TABLE_H

#include "symbol/symbol.h"
#include "../hash_table/hash_table.h"
#include <stdio.h>

typedef struct {
    HashTable *table;
    List *var_symbols; // final list of var_symbols that will be generated in the application by the code generator
} SymbolTable;

SymbolTable *init_symbol_table();

char *get_symbol_id(char *symbol_id, int scope_id);

Symbol *symbol_table_lookup(SymbolTable *table, char *id);

int symbol_table_insert(SymbolTable *table, SymbolType type, char *id, SymbolValue value, AstNode *initializer);

int symbol_table_remove(SymbolTable *table, char *id);

void symbol_table_dispose(SymbolTable *table);

#endif //INFINITY_COMPILER_SYMBOL_TABLE_H
