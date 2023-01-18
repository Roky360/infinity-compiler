#ifndef INFINITY_COMPILER_OPERATOR_APPLIERS_H
#define INFINITY_COMPILER_OPERATOR_APPLIERS_H

double apply_addition(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_subtraction(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_multiplication(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_division(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_power(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_modulus(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_factorial(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_logical_and(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_logical_or(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_not(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_equality(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_not_equal(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_greater_than(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_greater_equal(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_lower_than(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

double apply_lower_equal(double a, double b, char *left_op_placeholder, char *right_op_placeholder);

#endif //INFINITY_COMPILER_OPERATOR_APPLIERS_H
