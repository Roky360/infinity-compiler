#ifndef INFINITY_COMPILER_SCOPE_STACK_H
#define INFINITY_COMPILER_SCOPE_STACK_H

#include "../list/list.h"
#include "scope/scope.h"

typedef struct {
    List *scopes;
} ScopeStack;

ScopeStack *init_scope_stack();

void scope_stack_add_scope(ScopeStack *scope_s, Scope *new_scope);

void scope_stack_dispose(ScopeStack *scope_s);

#endif //INFINITY_COMPILER_SCOPE_STACK_H
