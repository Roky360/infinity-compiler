#include "symbol.h"
#include "../../logging/logging.h"
#include <stdlib.h>
#include <string.h>

Symbol *init_symbol(SymbolType type, SymbolValue value, AstNode *initializer) {
    Symbol *e = malloc(sizeof(Symbol));
    if (!e)
        throw_memory_allocation_error(COMPILER);
    e->type = type;
    e->value = value;
    e->initializer = initializer;
    if (type == VARIABLE) {
        switch (value.var_symbol.type) {
            case TYPE_CHAR:
            case TYPE_BOOL:
                e->value.var_symbol.var_size = BYTE;
                break;
            case TYPE_STRING:
            case TYPE_INT:
                e->value.var_symbol.var_size = DWORD;
                break;
            default:
                break;
        }
    }
    return e;
}

int compare_variable_signatures(VariableSymbol *var1, VariableSymbol *var2) {
    return strcmp(var1->var_name, var2->var_name);
}

int compare_function_signatures(FunctionSymbol *func1, FunctionSymbol *func2) {
    return strcmp(func1->func_name, func2->func_name);
}

void symbol_dispose(void *entry) {
    free((Symbol *) entry);
}
