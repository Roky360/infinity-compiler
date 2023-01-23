#include "symbol_table.h"
#include "../logging/logging.h"
#include "../config/globals.h"
#include "../io/io.h"

#include <stdlib.h>

SymbolTable *init_symbol_table() {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table)
        throw_memory_allocation_error(COMPILER);
    table->table = init_hash_table(SYMBOL_TABLE_SIZE, symbol_dispose);
    table->var_symbols = init_list(sizeof(Symbol *));

    return table;
}

char *get_symbol_id(char *symbol_id, int scope_id) {
    char *id;
    alsprintf(&id, "%s%d", symbol_id, scope_id);
    return id;
}

Symbol *symbol_table_lookup(SymbolTable *table, char *id) {
    return (Symbol *) hash_table_lookup(table->table, id);
}

// returns if insertion was successful
// if an entry with the same id exists, will return false (0)
int symbol_table_insert(SymbolTable *table, SymbolType type, char *id, SymbolValue value, AstNode *initializer) {
    Symbol *symbol = init_symbol(type, value, initializer);
    int success = hash_table_insert(table->table, id, symbol);
    if (success && type == VARIABLE) {
        list_push(table->var_symbols, symbol);
    }
    return success;
}

// if the item does not exist, return 0
// if the deletion was successful, return 1
int symbol_table_remove(SymbolTable *table, char *id) {
    return hash_table_remove(table->table, id);
}

void symbol_table_dispose(SymbolTable *table) {
    hash_table_dispose(table->table);
    free(table->var_symbols->items);
    free(table->var_symbols);
    free(table);
}
