#include "scope_stack.h"
#include <stdlib.h>

// initializes an empty scope stack with a global scope
ScopeStack *init_scope_stack() {
    ScopeStack *scope_s = malloc(sizeof(ScopeStack));
    scope_s->curr_scope_id = 0;
    scope_s->scopes = init_list(sizeof(Scope *));
    // init with global scope
    scope_stack_push_scope(scope_s);
    return scope_s;
}

// adds a new scope to the top of the stack with a new unique scope_id
void scope_stack_push_scope(ScopeStack *scope_s) {
    list_push(scope_s->scopes, init_scope(scope_s->curr_scope_id++));
}

// pops out the top scope from the stack
void scope_stack_pop_scope(ScopeStack *scope_s) {
    scope_dispose((Scope *) list_pop(scope_s->scopes));
}

// adds in identifier to the scope at the top of the stack
void scope_stack_add_identifier(ScopeStack *scope_s, char *id) {
    scope_add_identifier(((Scope *) list_get_last(scope_s->scopes)), id);
}

// searches for an identifier in the current or the parent scopes
// returns the identifier if found, or NULL if not found
char *scope_stack_lookup(ScopeStack *scope_s, char *id) {
    int i;
    char *e;
    for (i = scope_s->curr_scope_id - 1; i >= 0; i--) {
        e = (char *) hash_table_lookup(((Scope *) scope_s->scopes->items[i])->identifiers, id);
        if (e) // if found
            return e;
    }
    return NULL;
}

void scope_stack_dispose(ScopeStack *scope_s) {
    list_dispose(scope_s->scopes);
    free(scope_s);
}
