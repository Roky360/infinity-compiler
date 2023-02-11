#include "code_generator.h"

#ifndef INFINITY_COMPILER_BUILTIN_FUNCTION_GENERATORS_H
#define INFINITY_COMPILER_BUILTIN_FUNCTION_GENERATORS_H

void generate_print(CodeGenerator *generator, AstNode *node);

void generate_println(CodeGenerator *generator, AstNode *node);

#endif //INFINITY_COMPILER_BUILTIN_FUNCTION_GENERATORS_H
