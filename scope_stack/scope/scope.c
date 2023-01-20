#include "scope.h"
#include "../../logging/logging.h"
#include "../../config/globals.h"
#include <malloc.h>

Scope *init_scope(int scope_id) {
    Scope *s = malloc(sizeof(Scope));
    if (!s)
        throw_memory_allocation_error(COMPILER);
    s->id = scope_id;
    s->identifiers = init_hash_table(SCOPE_SIZE, NULL);
    return s;
}

int scope_add_identifier(Scope *scope, char *id) {
    return hash_table_insert(scope->identifiers, id, id);
}

void scope_dispose(Scope *scope) {
    hash_table_dispose(scope->identifiers);
    free(scope);
}
