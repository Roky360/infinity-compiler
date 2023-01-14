#include "variable.h"
#include <stdio.h>
#include <stdlib.h>

Variable *init_variable(char *name, LiteralValue *value) {
    Variable *var = (Variable *) malloc(sizeof(Variable));
    if (!var) {
        printf("Can't allocate memory for variable \"%s\".\n", name);
        exit(1);
    }
    var->name = name;
    var->value = value;
    return var;
}

void variable_dispose(Variable *var) {
    free(var->name);
    free(var->value);
    free(var);
}