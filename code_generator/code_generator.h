#ifndef INFINITY_COMPILER_CODE_GENERATOR_H
#define INFINITY_COMPILER_CODE_GENERATOR_H

#include "../ast/ast.h"

typedef struct CodeGenerator {
    List *symbols;
    AstNode *root;
} CodeGenerator;

CodeGenerator *init_code_generator(List *symbols, AstNode *root);

void *dispose_code_generator(CodeGenerator *generator);

#endif //INFINITY_COMPILER_CODE_GENERATOR_H
