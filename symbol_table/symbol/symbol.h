#ifndef INFINITY_COMPILER_SYMBOL_H
#define INFINITY_COMPILER_SYMBOL_H

#include "../../types/types.h"
#include "../../variable/variable.h"
#include "../../ast/ast.h"

typedef enum {
    BYTE,
    DWORD,
} VarSize;

typedef struct {
    char *func_name;
    List *arg_types;
    int returned; // if a return statement was met
} FunctionSymbol;

typedef struct {
    char *var_name;
    DataType type;
    VarSize var_size;
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
    SymbolValue value;
    AstNode *initializer; // A reference to the node where the symbol is initialized (used by the code generator)
} Symbol;

Symbol *init_symbol(SymbolType type, SymbolValue value, AstNode *initializer);

int compare_variable_signatures(VariableSymbol *var1, VariableSymbol *var2);

int compare_function_signatures(FunctionSymbol *func1, FunctionSymbol *func2);

void symbol_dispose(void *entry);

#endif //INFINITY_COMPILER_SYMBOL_H
