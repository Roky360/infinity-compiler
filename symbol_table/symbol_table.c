#include "symbol_table.h"
#include "../logging/logging.h"

#include <stdlib.h>
#include <string.h>

SymbolTable *init_symbol_table() {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table)
        throw_memory_allocation_error(COMPILER);
    table->items = calloc(TABLE_SIZE, sizeof(SymbolEntry *));
    if (!table->items)
        throw_memory_allocation_error(COMPILER);
    table->capacity = TABLE_SIZE;

    return table;
}

/**
 * Uses the Fowler–Noll–Vo hash function.
*/
unsigned int hash_func(char *id) {
    size_t hash = FNV_offset_basis;
    while (*id) {
        hash *= FNV_prime;
        hash ^= *id;
        id++;
    }
    // Incorporate the scope id into the hash calculation
//    hash *= FNV_prime;
//    hash ^= scope_id;
    return hash % TABLE_SIZE;
}

SymbolEntry *symbol_table_lookup(SymbolTable *table, char *id) {
    SymbolEntry *e = table->items[hash_func(id)];
    while (e) {
        if (strcmp(e->id, id) == 0)
            return e;
        e = e->next;
    }
    return NULL;
}

// returns if insertion was successful
// if an entry with the same id exists, will return false (0)
int symbol_table_insert(SymbolTable *table, SymbolType type, char *id, SymbolValue value, AstNode *initializer) {
    SymbolEntry *new_entry, *e;
    unsigned int idx;
    if (symbol_table_lookup(table, id) != NULL) {
        return 0;
    }

    idx = hash_func(id);
    new_entry = init_symbol_entry(type, id, value, initializer);
    e = table->items[idx];
    if (e == NULL) {
        table->items[idx] = new_entry;
    } else {
        while (e->next != NULL)
            e = e->next;
        e->next = new_entry;
    }

    return 1;
}

int symbol_table_delete(SymbolTable *table, char *id) {
    SymbolEntry *e, *prev;
    if (symbol_table_lookup(table, id) == NULL) {
        return 0;
    }

    e = table->items[hash_func(id)];
    if (e->next) {
        prev = e;
        while (strcmp(e->id, id) != 0) {
            prev = e;
            e = e->next;
        }
        prev->next = e->next;
    }
    symbol_entry_dispose(e);

    return 1;
}

void symbol_table_dispose(SymbolTable *table) {
    int i;
    SymbolEntry *e;
    for (i = 0; i < TABLE_SIZE; i++) {
        e = table->items[i];
        if (e) {
            symbol_entry_dispose_list(e);
        }
    }
    free(table);
}
