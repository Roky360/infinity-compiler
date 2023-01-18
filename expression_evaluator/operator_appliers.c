#include "operator_appliers.h"
#include "expression_evaluator.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

double _fact(double x) {
    return (x <= 1) ? 1 : x * _fact(x - 1);
}

double factorial(double x) {
    if (floor(x) != x) {
        fprintf(stderr, "Trying to perform factorial on a float number\n");
        exit(1);
    } else if (x < 0) {
        fprintf(stderr, "Trying to perform factorial on a negative number\n");
        exit(1);
    }
    return _fact(x);
}

double apply_addition(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a + b;
}

double apply_subtraction(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a - b;
}

double apply_multiplication(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a * b;
}

double apply_division(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    if (b == 0) {
        fprintf(stderr, "Attempting to divide by zero (%.2f / 0)\n", a);
        exit(1);
    }
    return a / b;
}

double apply_power(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return pow(a, b);
}

double apply_modulus(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return fmod(a, b);
}

double apply_factorial(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    if (strcmp(right_op_placeholder, FACT_OP_PLACEHOLDER) != 0) {
        fprintf(stderr, "Invalid expression\n");
        exit(1);
    }
    return factorial(a);
}

double apply_logical_and(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a && b;
}

double apply_logical_or(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a || b;
}

double apply_not(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    if (strcmp(left_op_placeholder, NOT_OP_PLACEHOLDER) != 0) {
        fprintf(stderr, "Invalid expression\n");
        exit(1);
    }
    return !b;
}

double apply_equality(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a == b;
}

double apply_not_equal(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a != b;
}

double apply_greater_than(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a > b;
}

double apply_greater_equal(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a >= b;
}

double apply_lower_than(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a < b;
}

double apply_lower_equal(double a, double b, char *left_op_placeholder, char *right_op_placeholder) {
    return a <= b;
}
