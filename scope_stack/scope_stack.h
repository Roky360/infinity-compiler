#ifndef INFINITY_COMPILER_SCOPE_STACK_H
#define INFINITY_COMPILER_SCOPE_STACK_H

#include "../list/list.h"
#include "scope/scope.h"

typedef struct ScopeStack {
    List *scopes;
    int curr_scope_id;
} ScopeStack;

ScopeStack *init_scope_stack();

void scope_stack_push_scope(ScopeStack *scope_s);

void scope_stack_pop_scope(ScopeStack *scope_s);

void scope_stack_add_identifier(ScopeStack *scope_s, char *id);

char *scope_stack_lookup(ScopeStack *scope_s, char *id);

void scope_stack_dispose(ScopeStack *scope_s);

#endif //INFINITY_COMPILER_SCOPE_STACK_H
