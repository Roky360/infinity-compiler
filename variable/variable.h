#ifndef INFINITY_COMPILER_VARIABLE_H
#define INFINITY_COMPILER_VARIABLE_H

#include "../types/types.h"

typedef struct {
    char *name;
    LiteralValue *value; // type and value_expr of the variable
} Variable;

Variable *init_variable(char *name, LiteralValue *value);

void variable_dispose(Variable *var);

#endif //INFINITY_COMPILER_VARIABLE_H
