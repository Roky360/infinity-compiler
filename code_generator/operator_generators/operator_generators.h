#ifndef INFINITY_COMPILER_OPERATOR_GENERATORS_H
#define INFINITY_COMPILER_OPERATOR_GENERATORS_H

#include "../instruction_generators.h"
#include "../../expression_evaluator/expression_evaluator.h"

void generate_op_addition(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_subtraction(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_multiplication(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_division(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_power(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_modulus(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_factorial(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_logical_and(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_logical_or(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_not(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_equality(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_not_equal(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_greater_than(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_greater_equal(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_lower_than(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);
void generate_op_lower_equal(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder, char *right_op_placeholder);

#endif //INFINITY_COMPILER_OPERATOR_GENERATORS_H
