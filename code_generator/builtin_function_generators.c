#include "builtin_function_generators.h"
#include "../io/io.h"
#include "instruction_generators.h"
#include "../expression_evaluator/expression_evaluator.h"
#include <stdlib.h>
#include <string.h>

void generate_print(CodeGenerator *generator, AstNode *node) {
    int i;
    char *buf;
    Expression *curr_arg_expr;
    StringSymbol *str_sym;

    for (i = 0; i < node->data.function_call.args->size; i++) {
        curr_arg_expr = &((AstNode *) node->data.function_call.args->items[i])->data.expression;

        if (curr_arg_expr->contains_variables) {
            if (curr_arg_expr->value->type == TYPE_STRING) {
                // string
                char *var_name = get_var_name_formatted(((Token *) curr_arg_expr->tokens->items[0])->value);
                char *eax = register_handler_request_register(generator->reg_handler, generator->fp, EAX);
                char *ebx = register_handler_request_register(generator->reg_handler, generator->fp, EBX);
                write_to_file(generator->fp, MOV, ebx, alsprintf(&buf, "[%s]", var_name));
                free(buf);
                write_to_file(generator->fp, MOVZX, eax, alsprintf(&buf, "byte [%s]", ebx));
                free(buf);
                write_to_file(generator->fp, PUSH, EAX);
                write_to_file(generator->fp, INC, ebx);
                write_to_file(generator->fp, PUSH, ebx);
                write_to_file(generator->fp, CALL, PRINT_PROC);

                register_handler_free_register(generator->reg_handler, generator->fp, eax);
                register_handler_free_register(generator->reg_handler, generator->fp, ebx);
            } else if (strstr(
                    "andornot>=<=!=",
                    ((ArithmeticToken *) curr_arg_expr->tokens->items[curr_arg_expr->tokens->size - 1])->value.op)) {
                // boolean
                char *false_label = generate_label(), *end_label = generate_label();
                generate_arithmetic_expression(generator, curr_arg_expr);
                write_to_file(generator->fp, CMP, EXPR_RES_REG, "0");
                write_to_file(generator->fp, JE, false_label);
                write_to_file(generator->fp, PUSH, "4"); // push len of true
                write_to_file(generator->fp, PUSH, TRUE_STR_VAR); // push true
                write_to_file(generator->fp, CALL, PRINT_PROC);
                write_to_file(generator->fp, JMP, end_label);
                write_to_file(generator->fp, LABEL_DEF, false_label);
                write_to_file(generator->fp, PUSH, "5"); // push len of false
                write_to_file(generator->fp, PUSH, FALSE_STR_VAR); // push false
                write_to_file(generator->fp, CALL, PRINT_PROC);
                write_to_file(generator->fp, LABEL_DEF, end_label);
            } else if (curr_arg_expr->tokens->size == 1 &&
                       ((ArithmeticToken *) curr_arg_expr->tokens->items[0])->original_tok->type == CHAR) {
                // char variable
                write_to_file(generator->fp, PUSH, alsprintf(&buf, "dword '%s'",
                                                             ((ArithmeticToken *) curr_arg_expr->tokens->items[0])->original_tok->value));
                write_to_file(generator->fp, CALL, PRINT_CHAR_PROC);
                free(buf);
            } else {
                // int
                generate_arithmetic_expression(generator, curr_arg_expr);
                write_to_file(generator->fp, PUSH, EXPR_RES_REG);
                write_to_file(generator->fp, CALL, PRINT_INT_PROC);
            }
        } else {
            // not containing variables
            if (curr_arg_expr->value->type == TYPE_STRING) {
                str_sym = string_repository_lookup(generator->symbol_table->str_repo,
                                                   curr_arg_expr->value->value.string_value);
                write_to_file(generator->fp, PUSH, alsprintf(&buf, "%d", str_sym->length)); // buf
                write_to_file(generator->fp, PUSH, alsprintf(&buf, "%s+1", str_sym->symbol_name)); // count
                write_to_file(generator->fp, CALL, PRINT_PROC);
                free(buf);
            } else if (curr_arg_expr->tokens->size == 1 &&
                       ((ArithmeticToken *) curr_arg_expr->tokens->items[0])->original_tok->type == CHAR) {
                // print char
                write_to_file(generator->fp, PUSH, alsprintf(&buf, "dword '%s'",
                                                             ((ArithmeticToken *) curr_arg_expr->tokens->items[0])->original_tok->value));
                write_to_file(generator->fp, CALL, PRINT_CHAR_PROC);
                free(buf);
            } else {
                write_to_file(generator->fp, PUSH,
                              alsprintf(&buf, "%d", (int) curr_arg_expr->value->value.double_value));
                write_to_file(generator->fp, CALL, PRINT_INT_PROC);
                free(buf);
            }
        }
    }
}

void generate_println(CodeGenerator *generator, AstNode *node) {
    generate_print(generator, node);
    write_to_file(generator->fp, CALL, PRINT_NEW_LINE_PROC);
}

void generate_exit(CodeGenerator *generator, AstNode *node) {
    char *buf;
    Expression *arg_expr = &((AstNode *) node->data.function_call.args->items[0])->data.expression;
    if (arg_expr->contains_variables) {
        generate_arithmetic_expression(generator, arg_expr);
        write_to_file(generator->fp, PUSH, EXPR_RES_REG);
    } else {
        write_to_file(generator->fp, PUSH, alsprintf(&buf, "%d",
                                                     (int) (arg_expr->value->value.double_value)));
        free(buf);
    }
    write_to_file(generator->fp, CALL, EXIT_PROC);
}
