#ifndef INFINITY_COMPILER_SYMBOL_TABLE_H
#define INFINITY_COMPILER_SYMBOL_TABLE_H

#include "symbol_entry/symbol_entry.h"
#include <stdio.h>

#define FNV_offset_basis (size_t)14695981039346656037
#define FNV_prime 1099511628211

#define TABLE_SIZE 199

typedef struct {
    SymbolEntry **items;
    unsigned int capacity;
} SymbolTable;

SymbolTable *init_symbol_table();

unsigned int hash_func(char *id);

SymbolEntry *symbol_table_lookup(SymbolTable *table, char *id);

int symbol_table_insert(SymbolTable *table, SymbolType type, char *id, SymbolValue value, AstNode *initializer);

int symbol_table_delete(SymbolTable *table, char *id);

void symbol_table_dispose(SymbolTable *table);

#endif //INFINITY_COMPILER_SYMBOL_TABLE_H
