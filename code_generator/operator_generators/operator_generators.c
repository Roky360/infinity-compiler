#include "operator_generators.h"
#include "../../io/io.h"
#include <string.h>
#include <stdlib.h>

void generate_op_addition(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                          char *right_op_placeholder, int is_last) {
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, ADD, reg_a, reg_b);
    if (!is_last)
        write_to_file(generator->fp, PUSH, reg_a);
}

void generate_op_subtraction(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                             char *right_op_placeholder, int is_last) {
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, SUB, reg_a, reg_b);
    if (!is_last)
        write_to_file(generator->fp, PUSH, reg_a);
}

void generate_op_multiplication(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                                char *right_op_placeholder, int is_last) {
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, IMUL, reg_b);
    if (!is_last)
        write_to_file(generator->fp, PUSH, reg_a);
}

void generate_op_division(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                          char *right_op_placeholder, int is_last) {
    char *edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    char *ok_label = generate_label();
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_b, "0");
    write_to_file(generator->fp, JNE, ok_label);
    write_to_file(generator->fp, CALL, EXIT_ZERO_DIV_PROC); // exit on zero division
    write_to_file(generator->fp, LABEL_DEF, ok_label);
    write_to_file(generator->fp, XOR, edx, edx);
    write_to_file(generator->fp, IDIV, reg_b);
    if (!is_last)
        write_to_file(generator->fp, PUSH, reg_a);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
    free(ok_label);
}

void generate_op_power(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                       char *right_op_placeholder, int is_last) {
    write_to_file(generator->fp, CALL, POWER_PROC);
    if (!is_last)
        write_to_file(generator->fp, PUSH, EAX);
}

void generate_op_modulus(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                         char *right_op_placeholder, int is_last) {
    char *edx;
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, XOR, edx, edx);
    write_to_file(generator->fp, IDIV, reg_b);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
}

void generate_op_factorial(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                           char *right_op_placeholder, int is_last) {
    // CHECK FACTORIAL INPUT
    if (strcmp(right_op_placeholder, FACT_OP_PLACEHOLDER) != 0) {
        fprintf(stderr, "Invalid expression\n");
        exit(1);
    }
    write_to_file(generator->fp, CALL, FACT_PROC);
    if (!is_last)
        write_to_file(generator->fp, PUSH, EAX);
}

void generate_op_logical_and(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                             char *right_op_placeholder, int is_last) {
    char *false_label = generate_label(), *done_label = generate_label();
    char *edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, MOV, edx, "1");
    write_to_file(generator->fp, CMP, reg_a, "0");
    write_to_file(generator->fp, JE, false_label);
    write_to_file(generator->fp, CMP, reg_b, "0");
    write_to_file(generator->fp, JE, false_label);
    write_to_file(generator->fp, JMP, done_label);
    write_to_file(generator->fp, LABEL_DEF, false_label);
    write_to_file(generator->fp, MOV, edx, "0");
    write_to_file(generator->fp, LABEL_DEF, done_label);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
    free(false_label);
    free(done_label);
}

void generate_op_logical_or(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                            char *right_op_placeholder, int is_last) {
    char *done_label = generate_label();
    char *edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_a, "0");
    write_to_file(generator->fp, SETNE, DL);
    write_to_file(generator->fp, JNE, done_label);
    write_to_file(generator->fp, CMP, reg_b, "0");
    write_to_file(generator->fp, SETNE, DL);
    write_to_file(generator->fp, LABEL_DEF, done_label);
    write_to_file(generator->fp, MOVSX, edx, DL);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
    free(done_label);
}

void generate_op_not(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                     char *right_op_placeholder, int is_last) {
    char *edx;
    if (strcmp(left_op_placeholder, NOT_OP_PLACEHOLDER) != 0) {
        fprintf(stderr, "Invalid expression\n");
        exit(1);
    }
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_a, "0");
    edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, SETZ, DL);
    write_to_file(generator->fp, MOVSX, edx, DL);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
}

void generate_op_equality(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                          char *right_op_placeholder, int is_last) {
    char *edx;
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_a, reg_b);
    edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, SETE, DL);
    write_to_file(generator->fp, MOVZX, edx, DL);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
}

void generate_op_not_equal(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                           char *right_op_placeholder, int is_last) {
    char *edx;
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_a, reg_b);
    edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, SETNE, DL);
    write_to_file(generator->fp, MOVZX, edx, DL);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
}

void generate_op_greater_than(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                              char *right_op_placeholder, int is_last) {
    char *edx;
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_a, reg_b);
    edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, SETG, DL);
    write_to_file(generator->fp, MOVZX, edx, DL);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
}

void generate_op_greater_equal(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                               char *right_op_placeholder, int is_last) {
    char *edx;
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_a, reg_b);
    edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, SETGE, DL);
    write_to_file(generator->fp, MOVZX, edx, DL);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
}

void generate_op_lower_than(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                            char *right_op_placeholder, int is_last) {
    char *edx;
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_a, reg_b);
    edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, SETL, DL);
    write_to_file(generator->fp, MOVZX, edx, DL);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
}

void generate_op_lower_equal(CodeGenerator *generator, char *reg_a, char *reg_b, char *left_op_placeholder,
                             char *right_op_placeholder, int is_last) {
    char *edx;
    write_to_file(generator->fp, POP, reg_b);
    write_to_file(generator->fp, POP, reg_a);
    write_to_file(generator->fp, CMP, reg_a, reg_b);
    edx = register_handler_request_register(generator->reg_handler, generator->fp, EDX);
    write_to_file(generator->fp, SETLE, DL);
    write_to_file(generator->fp, MOVZX, edx, DL);
    if (!is_last)
        write_to_file(generator->fp, PUSH, edx);
    else
        write_to_file(generator->fp, MOV, reg_a, edx);
    register_handler_free_register(generator->reg_handler, generator->fp, edx);
}
