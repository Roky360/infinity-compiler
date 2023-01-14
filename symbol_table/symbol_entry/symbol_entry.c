#include "symbol_entry.h"
#include "../../logging/logging.h"
#include "../../io/io.h"
#include <stdlib.h>
#include <string.h>

SymbolEntry *init_symbol_entry(SymbolType type, char *id, SymbolValue value, AstNode *initializer) {
    SymbolEntry *e = malloc(sizeof(SymbolEntry));
    if (!e)
        throw_memory_allocation_error(COMPILER);
    e->type = type;
    e->id = id;
    e->value = value;
    e->initializer = initializer;
    e->next = NULL;
    return e;
}

int compare_variable_signatures(VariableSymbol *var1, VariableSymbol *var2) {
    return strcmp(var1->var_name, var2->var_name);
}

int compare_function_signatures(FunctionSymbol *func1, FunctionSymbol *func2) {
    return strcmp(func1->func_name, func2->func_name);
}

void symbol_entry_dispose(SymbolEntry *entry) {
    free(entry->next);
    free(entry->id);
    free(entry);
}

void symbol_entry_dispose_list(SymbolEntry *entry) {
    free(entry->next);
    free(entry->id);
    if (entry->next)
        symbol_entry_dispose(entry->next);
    free(entry);
}
