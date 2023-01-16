#ifndef INFINITY_COMPILER_SCOPE_H
#define INFINITY_COMPILER_SCOPE_H

#include "../../hash_table/hash_table.h"
#include "../../symbol_table/symbol/symbol.h"

typedef struct Scope {
    int id;
    HashTable *identifiers;
} Scope;

Scope *init_scope(int scope_id);

int scope_add_identifier(Scope *scope, char *id);

void dispose_identifier(void *item);

void scope_dispose(Scope *scope);

#endif //INFINITY_COMPILER_SCOPE_H
