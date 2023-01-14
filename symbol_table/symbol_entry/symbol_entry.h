#ifndef INFINITY_COMPILER_SYMBOL_ENTRY_H
#define INFINITY_COMPILER_SYMBOL_ENTRY_H

#include "../../types/types.h"
#include "../../variable/variable.h"
#include "../../ast/ast.h"

typedef struct {
    char *func_name;
    List *arg_types;
    int returned; // if a return statement was met
} FunctionSymbol;

typedef struct {
    char *var_name;
    DataType type;
} VariableSymbol;

typedef enum {
    VARIABLE,
    FUNCTION
} SymbolType;

typedef union {
    VariableSymbol var_symbol;
    FunctionSymbol func_symbol;
} SymbolValue;

typedef struct SymbolEntry {
    SymbolType type;
    char *id;
    SymbolValue value;
    AstNode *initializer; // A reference to the node where the symbol is initialized (used by the code generator)
    struct SymbolEntry *next;
} SymbolEntry;

SymbolEntry *init_symbol_entry(SymbolType type, char *id, SymbolValue value, AstNode *initializer);

int compare_variable_signatures(VariableSymbol *var1, VariableSymbol *var2);

int compare_function_signatures(FunctionSymbol *func1, FunctionSymbol *func2);

void symbol_entry_dispose(SymbolEntry *entry);

void symbol_entry_dispose_list(SymbolEntry *entry);

#endif //INFINITY_COMPILER_SYMBOL_ENTRY_H
