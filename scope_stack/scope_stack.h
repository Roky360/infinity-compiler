#ifndef INFINITY_COMPILER_SCOPE_STACK_H
#define INFINITY_COMPILER_SCOPE_STACK_H

#include "../list/list.h"
#include "scope/scope.h"

typedef struct ScopeStack {
    List *scopes;
    int curr_scope_id;
} ScopeStack;

/// Initializes an empty scope stack
/// \return
ScopeStack *init_scope_stack();

/// Adds a new scope to the scope stack
/// \param scope_s
void scope_stack_push_scope(ScopeStack *scope_s);

/// Pops the most-top scope from a scope stack
/// \param scope_s
void scope_stack_pop_scope(ScopeStack *scope_s);

/// Adds an identifier to the top-most (current) scope.
/// \param scope_s
/// \param id The identifier to add
void scope_stack_add_identifier(ScopeStack *scope_s, char *id);

/// Searches for an identifier in the scope stack. Searches in all the scopes in the scope stack
/// \param scope_s
/// \param id The identifier to search
/// \return The id if found, NULL if not found.
char *scope_stack_lookup(ScopeStack *scope_s, char *id);

void scope_stack_dispose(ScopeStack *scope_s);

#endif //INFINITY_COMPILER_SCOPE_STACK_H
