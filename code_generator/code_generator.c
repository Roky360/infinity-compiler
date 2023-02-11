#include "code_generator.h"
#include "../io/io.h"
#include "../logging/logging.h"
#include "instruction_generators.h"
#include "../config/table_initializers.h"
#include "../expression_evaluator/expression_evaluator.h"
#include "../symbol_table/string_repository/string_symbol.h"
#include <stdlib.h>
#include <string.h>

CodeGenerator *
init_code_generator(SymbolTable *symbol_table, AstNode *root, AstNode *starting_point, char *target_path) {
    CodeGenerator *generator = malloc(sizeof(CodeGenerator));
    if (!generator)
        throw_memory_allocation_error(CODE_GENERATOR);

    generator->symbol_table = symbol_table;
    generator->root = root;
    generator->starting_point = starting_point;
    generator->reg_handler = init_register_handler();
    generator->target_path = target_path;

    return generator;
}

void dispose_code_generator(CodeGenerator *generator) {
    register_handler_dispose(generator->reg_handler);
    free(generator);
}

void code_generator_generate(CodeGenerator *generator) {
    generator->fp = fopen(generator->target_path, "w");
    if (!generator->fp) // add path to error msg
        log_error(CODE_GENERATOR, "Could not create output file.");

    // generate...
    generate_data_segment(generator);
    generate_bss_segment(generator);
    generate_code_segment(generator);

    fclose(generator->fp);
}

void generate_data_segment(CodeGenerator *generator) {
    int i;
    List *string_symbols = generator->symbol_table->str_repo->lst;
    StringSymbol *curr_sym;

    write_to_file(generator->fp, SECTION, "data");
    write_to_file(generator->fp, "\tout_buf times 11 db 0\n");
    write_to_file(generator->fp, "\tout_buf_len equ $-out_buf\n");
    write_to_file(generator->fp, "\tchar_buf db 0\n");
    write_to_file(generator->fp, "\tnew_line_chr db 13\n");
    write_to_file(generator->fp, "\ttrue_str db \"true\"\n");
    write_to_file(generator->fp, "\tfalse_str db \"false\"\n");
    write_to_file(generator->fp, "\n");

    // define string literals
    for (i = 0; i < string_symbols->size; i++) {
        curr_sym = (StringSymbol *) string_symbols->items[i];
        write_to_file(generator->fp, "\t%s db ", curr_sym->symbol_name);
        write_to_file(generator->fp, "%d, ", curr_sym->length); // write string length
        for (int j = 0; j < curr_sym->length; j++) {
            if (strchr("\n\t", curr_sym->value[j])) { // if escape character
                write_to_file(generator->fp, "%d, ", curr_sym->value[j]);
            } else {
                write_to_file(generator->fp, "'%c', ", curr_sym->value[j]);
            }
        }
        write_to_file(generator->fp, "0\n");
    }

    write_to_file(generator->fp, "\n");
}

void generate_bss_segment(CodeGenerator *generator) {
    int i;
    char *var_type, *format;
    Symbol *symbol;

    write_to_file(generator->fp, SECTION, "bss");

    for (i = 0; i < generator->symbol_table->var_symbols->size; i++) {
        symbol = (Symbol *) generator->symbol_table->var_symbols->items[i];

        switch (symbol->value.var_symbol.type) {
            case TYPE_BOOL:
            case TYPE_CHAR:
                var_type = RESB;
                break;
            case TYPE_STRING:
            case TYPE_INT:
                var_type = RESD;
                break;
            default:
                continue;
        }
        // define the variable in the data segment
        write_to_file(generator->fp, alsprintf(&format, "\t%%s%s", var_type),
                      get_var_name_formatted(symbol->value.var_symbol.var_name), 1);
        free(format);
        // TODO: maybe add len of each var
    }
    write_to_file(generator->fp, "\n");
}

void generate_code_segment(CodeGenerator *generator) {
    write_to_file(generator->fp, GLOBAL, ENTRY_POINT_NAME);
    write_to_file(generator->fp, SECTION, "text");
    write_to_file(generator->fp, LABEL_DEF, ENTRY_POINT_NAME);

    // call main function
    write_to_file(generator->fp, CALL,
                  get_proc_name_formatted(generator->starting_point->data.function_definition.func_name));

    // exit
    if (generator->starting_point->data.function_definition.returnType != TYPE_INT) // move return code to EBX
        write_to_file(generator->fp, XOR, EBX, EBX); // return 0
    else
        write_to_file(generator->fp, MOV, EBX, EAX); // return whatever is returned from main
    write_to_file(generator->fp, MOV, EAX, "1");
    write_to_file(generator->fp, SYSCALL_80H);
    write_to_file(generator->fp, "\n");

    // generate functions
    generate_block(generator, generator->root->data.compound.children);
    // write helper procedures
    write_to_file(generator->fp, read_file(INCLUDE_ASM_PATH));
}

char *get_variable_size_prefix(CodeGenerator *generator, char *var_name) {
    Symbol *symbol = symbol_table_lookup(generator->symbol_table, var_name);
    if (!symbol)
        return NULL;
    switch (symbol->value.var_symbol.var_size) {
        case BYTE:
            return "byte ";
        case DWORD:
            return "dword ";
    }
}

void code_generator_apply_assignment(CodeGenerator *generator, DataType var_type, char *var_name, char *reg) {
    char *inst_format, *ref_format, *reg_low_byte;
    alsprintf(&ref_format, "[%s]", var_name);

    switch (var_type) {
        case TYPE_BOOL:
        case TYPE_CHAR:
            if (register_handler_is_register_byte(reg)) {
                write_to_file(generator->fp, MOV, ref_format, reg);
            } else if ((reg_low_byte = register_handler_get_lower_byte(reg)) != NULL) {
                write_to_file(generator->fp, MOV, ref_format, reg_low_byte);
            } else {
                write_to_file(generator->fp, MOV, ref_format, alsprintf(&inst_format, "byte %s", reg));
                free(inst_format);
            }
            break;
        case TYPE_STRING:
        case TYPE_INT:
            write_to_file(generator->fp, MOV, ref_format, reg);
            break;
        default:
            break;
    }

    register_handler_free_register(generator->reg_handler, generator->fp, reg);
}

// returns whether a return statement has met, for the use of the parent node, if it's a function
int generate_block(CodeGenerator *generator, List *block) {
    int i, returned = 0;
    for (i = 0; i < block->size; i++) {
        generate_statement(generator, (AstNode *) block->items[i]);
        if (((AstNode *) block->items[i])->type == AST_RETURN_STATEMENT)
            returned = 1;
    }
    return returned;
}

void generate_statement(CodeGenerator *generator, AstNode *node) {
    char *id_ptr;
    void (*generator_func)(CodeGenerator *, AstNode *);

    generator_func = hash_table_lookup(statement_to_generator_map, alsprintf(&id_ptr, "%d", node->type));
    free(id_ptr);
    if (generator_func) {
        generator_func(generator, node);
    }
}

void generate_complicated_arithmetic_expression(CodeGenerator *generator, List *postfix_expr_lst) {
    int i;
    List *stack;
    ArithmeticToken *curr_token, *result_token, *left_token, *right_token;
    double (*applier_func)(CodeGenerator *, char *, char *, char *, char *, int);
    char *eax, *ebx, *format;

    stack = init_list(sizeof(ArithmeticToken *));
    eax = register_handler_request_register(generator->reg_handler, generator->fp, EXPR_RES_REG);
    ebx = register_handler_request_register(generator->reg_handler, generator->fp, EBX);

    for (i = 0; i < postfix_expr_lst->size; i++) {
        curr_token = postfix_expr_lst->items[i];

        if (curr_token->type == NUMBER) {
            list_push(stack, curr_token);
            write_to_file(generator->fp, MOV, eax, alsprintf(&format, "%d", (int) curr_token->value.number));
            if (i != postfix_expr_lst->size - 1) // if not last element
                write_to_file(generator->fp, PUSH, eax);
            free(format);
        } else if (curr_token->type == PLACEHOLDER) {
            list_push(stack, curr_token);
        } else if (curr_token->type == VAR) {
            list_push(stack, curr_token);
            if (symbol_table_lookup(generator->symbol_table, curr_token->value.var)->value.var_symbol.var_size ==
                BYTE) {
                write_to_file(generator->fp, MOVSX, eax,
                              alsprintf(&format, "byte [%s]", get_var_name_formatted(curr_token->value.var)));
            } else {
                write_to_file(generator->fp, MOV, eax,
                              alsprintf(&format, "[%s]", get_var_name_formatted(curr_token->value.var)));
            }
            if (i != postfix_expr_lst->size - 1) // if not last element
                write_to_file(generator->fp, PUSH, eax);
            free(format);
        } else if (curr_token->type == OPERATOR) {
            result_token = init_empty_arithmetic_token();
            right_token = list_pop(stack);
            left_token = list_pop(stack);
            if (!right_token || !left_token) {
                fprintf(stderr, "Missing operands\n");
                exit(1);
            }

            applier_func = hash_table_lookup(operator_to_generator_map, curr_token->value.op);
            if (applier_func) {
                applier_func(generator, eax, ebx, left_token->type == PLACEHOLDER ? left_token->value.op : "",
                             right_token->type == PLACEHOLDER ? right_token->value.op : "",
                             i == postfix_expr_lst->size - 1);
            } else {
                fprintf(stderr, "Invalid operator: %s\n", curr_token->value.op);
                exit(1);
            }

            result_token->type = NUMBER;
            result_token->value.number = 1;
            list_push(stack, result_token);
        }
    }
    list_pop(stack); // pop last element - the "result"
    if (!list_is_empty(stack)) {
        fprintf(stderr, "Invalid expression\n");
        exit(1);
    }

    list_dispose(stack);
    register_handler_free_register(generator->reg_handler, generator->fp, eax);
    register_handler_free_register(generator->reg_handler, generator->fp, ebx);
}

// calculates complicated expressions that contains variables and stores the result in EAX
void generate_arithmetic_expression(CodeGenerator *generator, Expression *expr) {
    char *eax, *expr_value_str;

    if (expr->contains_variables) {
        generate_complicated_arithmetic_expression(generator, expr->tokens);
    } else {
        eax = register_handler_request_register(generator->reg_handler, generator->fp, EXPR_RES_REG);
        if (expr->value->type == TYPE_STRING) {
            write_to_file(generator->fp, MOV, eax, string_repository_lookup(generator->symbol_table->str_repo,
                                                                            expr->value->value.string_value)->symbol_name);
        } else {
            write_to_file(generator->fp, MOV, eax,
                          alsprintf(&expr_value_str, "%d", (int) expr->value->value.double_value));
            free(expr_value_str);
        }
        register_handler_free_register(generator->reg_handler, generator->fp, eax);
    }
}

void generate_variable_declaration(CodeGenerator *generator, AstNode *node) {
    char *eax, *var_name;
    var_name = get_var_name_formatted(node->data.variable_declaration.var->name);

    generate_arithmetic_expression(generator, &node->data.variable_declaration.value->data.expression);
    eax = register_handler_request_register(generator->reg_handler, generator->fp, EXPR_RES_REG);
    code_generator_apply_assignment(generator, node->data.variable_declaration.var->value->type, var_name, eax);

    write_to_file(generator->fp, "\n");
}

void generate_assignment(CodeGenerator *generator, AstNode *node) {
    char *eax, *var_name;
    Symbol *target_var;
    var_name = get_var_name_formatted(node->data.assignment.dst_variable->value);
    target_var = (Symbol *) hash_table_lookup(generator->symbol_table->table,
                                              node->data.assignment.dst_variable->value);

    generate_arithmetic_expression(generator, &node->data.assignment.expression->data.expression);
    eax = register_handler_request_register(generator->reg_handler, generator->fp, EXPR_RES_REG);
    code_generator_apply_assignment(generator, target_var->value.var_symbol.type, var_name, eax);

    write_to_file(generator->fp, "\n");
}

void generate_function(CodeGenerator *generator, AstNode *node) {
    int returned, i;
    char *proc_name = get_proc_name_formatted(node->data.function_definition.func_name), *arg_buf;
    char *eax;
    Variable *curr_arg;

    // function name label
    write_to_file(generator->fp, GLOBAL, proc_name);
    write_to_file(generator->fp, LABEL_DEF, proc_name);
    // if function accepts arguments, use EBP
    if (node->data.function_definition.args->size > 0) {
        write_to_file(generator->fp, PUSH, EBP);
        write_to_file(generator->fp, MOV, EBP, ESP);
        eax = register_handler_request_register(generator->reg_handler, generator->fp, EAX);
        // initialize arguments
        for (i = 0; i < node->data.function_definition.args->size; i++) {
            curr_arg = (Variable *) node->data.function_definition.args->items[i];
            write_to_file(generator->fp, MOV, eax, alsprintf(&arg_buf, "[ebp+%d]", 8 + 4 * i));
            free(arg_buf);
            if (symbol_table_lookup(generator->symbol_table, curr_arg->name)->value.var_symbol.var_size == BYTE) {
                // byte
                write_to_file(generator->fp, MOV,
                              alsprintf(&arg_buf, "byte [%s]", get_var_name_formatted(curr_arg->name)),
                              AL);
            } else {
                // dword
                write_to_file(generator->fp, MOV,
                              alsprintf(&arg_buf, "dword [%s]", get_var_name_formatted(curr_arg->name)),
                              eax);
            }
        }
        register_handler_free_register(generator->reg_handler, generator->fp, eax);
        write_to_file(generator->fp, "\n");
    }
    // generate body
    returned = generate_block(generator, node->data.function_definition.body);

    // return manually if there is no return statement
    if (!returned) {
        if (node->data.function_definition.args->size == 0) {
            // no args
            write_to_file(generator->fp, RET);
        } else {
            write_to_file(generator->fp, POP, EBP);
            write_to_file(generator->fp, RET_NUM, node->data.function_definition.args->size * 4);
        }
    }
    write_to_file(generator->fp, COMMENT, proc_name);

    write_to_file(generator->fp, "\n");
    free(proc_name);
}

void generate_function_call(CodeGenerator *generator, AstNode *node) {
    int i;
    void (*builtin_func)(CodeGenerator *, AstNode *);

    builtin_func = hash_table_lookup(builtin_function_to_generator_map, node->data.function_call.func_name);
    if (builtin_func) { // builtin function
        builtin_func(generator, node);
    } else { // other function
        for (i = node->data.function_call.args->size - 1; i >= 0; i--) {
            generate_arithmetic_expression(generator,
                                           &((AstNode *) node->data.function_call.args->items[i])->data.expression);
            write_to_file(generator->fp, PUSH, EXPR_RES_REG); // push each argument
        }
        write_to_file(generator->fp, CALL,
                      get_proc_name_formatted(node->data.function_call.func_name)); // call function
    }
    write_to_file(generator->fp, "\n");
}

void generate_if_statement(CodeGenerator *generator, AstNode *node) {
    char *false_label = generate_label(), *done_if = generate_label();
    // evaluate the condition, result in EAX
    generate_arithmetic_expression(generator, &node->data.if_statement.condition->data.expression);

    write_to_file(generator->fp, CMP, EXPR_RES_REG, "0");
    write_to_file(generator->fp, JE, false_label);
    // generate body
    generate_block(generator, node->data.if_statement.body_node);
    write_to_file(generator->fp, JMP, done_if);
    // else block
    write_to_file(generator->fp, LABEL_DEF, false_label);
    generate_block(generator, node->data.if_statement.else_node);
    write_to_file(generator->fp, LABEL_DEF, done_if);

    write_to_file(generator->fp, "\n");
    free(false_label);
    free(done_if);
}

void generate_simple_loop(CodeGenerator *generator, AstNode *node) {
    char *ecx;
    char *loop_count, *loop_label, *end_loop_label;
    ecx = register_handler_request_register(generator->reg_handler, generator->fp, ECX);
    loop_label = generate_label();

    // if end expression needs evaluation
    if (node->data.loop.end->contains_variables) {
        end_loop_label = generate_label();
        generate_arithmetic_expression(generator, node->data.loop.end);
        write_to_file(generator->fp, MOV, ecx, EAX);
        write_to_file(generator->fp, CMP, ecx, "0");
        write_to_file(generator->fp, JNG, end_loop_label);
    } else {
        write_to_file(generator->fp, MOV, ecx,
                      alsprintf(&loop_count, "%d", (int) node->data.loop.end->value->value.double_value));
    }
    write_to_file(generator->fp, LABEL_DEF, loop_label);

    // generate loop body
    generate_block(generator, node->data.loop.body);

    write_to_file(generator->fp, LOOP, loop_label);
    if (node->data.loop.end->contains_variables) {
        write_to_file(generator->fp, LABEL_DEF, end_loop_label);
        free(end_loop_label);
    }
    write_to_file(generator->fp, "\n");

    register_handler_free_register(generator->reg_handler, generator->fp, ecx);
    free(loop_label);
}

void generate_loop_with_counter(CodeGenerator *generator, AstNode *node) {
    char *loop_label = generate_label(), *loop_end_label = generate_label(), *inc_label = generate_label();
    char *loop_counter = node->data.loop.loop_counter_name;
    char *form2, *loop_counter_form = alsprintf(&loop_counter_form, "dword [%s]", get_var_name_formatted(loop_counter));
    int loop_range_is_expression = 0;
    char *edi = register_handler_request_register(generator->reg_handler, generator->fp, EDI);

    // if end expression needs evaluation
    if (node->data.loop.end->contains_variables) {
        loop_range_is_expression = 1;
        generate_arithmetic_expression(generator, node->data.loop.end);

        write_to_file(generator->fp, MOV, edi, EAX); // store `end` in EDI
        write_to_file(generator->fp, CMP, loop_counter_form, edi);
    } else {
        write_to_file(generator->fp, MOV, edi,
                      alsprintf(&form2, "%d",
                                (int) node->data.loop.end->value->value.double_value)); // store `end` in EDI
        free(form2);
    }
    // if start expression needs evaluation
    if (node->data.loop.start->contains_variables) {
        loop_range_is_expression = 1;
        // evaluate start expression, result in EAX
        generate_arithmetic_expression(generator, node->data.loop.start);
        write_to_file(generator->fp, MOV, loop_counter_form, EAX);
    } else {
        write_to_file(generator->fp, MOV, loop_counter_form,
                      alsprintf(&form2, "%d", (int) node->data.loop.start->value->value.double_value));
        free(form2);
    }

    write_to_file(generator->fp, LABEL_DEF, loop_label);

    write_to_file(generator->fp, CMP, loop_counter_form, edi);
    write_to_file(generator->fp, JE, loop_end_label);
    write_to_file(generator->fp, "\n");
    // generate loop body
    generate_block(generator, node->data.loop.body);
    // if the start or the end is an expression
    if (loop_range_is_expression) {
        write_to_file(generator->fp, CMP, loop_counter_form, edi);
        write_to_file(generator->fp, JL, inc_label);
        write_to_file(generator->fp, DEC, loop_counter_form);
        write_to_file(generator->fp, JMP, loop_label);
        write_to_file(generator->fp, LABEL_DEF, inc_label);
        write_to_file(generator->fp, INC, loop_counter_form);
        write_to_file(generator->fp, JMP, loop_label);
    } else {
        write_to_file(generator->fp, node->data.loop.forward ? INC : DEC, loop_counter_form);
        write_to_file(generator->fp, JMP, loop_label);
    }
    write_to_file(generator->fp, LABEL_DEF, loop_end_label);

    free(loop_counter_form);
    register_handler_free_register(generator->reg_handler, generator->fp, edi);
}

void generate_loop(CodeGenerator *generator, AstNode *node) {
    node->data.loop.loop_counter_name
    ? generate_loop_with_counter(generator, node)
    : generate_simple_loop(generator, node);
}

void generate_while_loop(CodeGenerator *generator, AstNode *node) {
    char *while_label = generate_label(), *end_loop = generate_label();
    write_to_file(generator->fp, LABEL_DEF, while_label);
    generate_arithmetic_expression(generator, &node->data.while_loop.condition->data.expression);
    write_to_file(generator->fp, CMP, EAX, "0");
    write_to_file(generator->fp, JE, end_loop);
    write_to_file(generator->fp, "\n");
    // generate body
    generate_block(generator, node->data.while_loop.body);

    write_to_file(generator->fp, JMP, while_label);
    write_to_file(generator->fp, LABEL_DEF, end_loop);

    free(while_label);
    free(end_loop);
}

void generate_return_statement(CodeGenerator *generator, AstNode *node) {
    unsigned int arg_count = node->data.return_statement.parent_function_arg_count;

    if (node->data.return_statement.value_expr->data.expression.value->type != TYPE_VOID) {
        generate_arithmetic_expression(generator, &node->data.return_statement.value_expr->data.expression);
    }

    if (arg_count == 0) {
        // no args
        write_to_file(generator->fp, RET);
    } else {
        write_to_file(generator->fp, POP, EBP);
        write_to_file(generator->fp, RET_NUM, arg_count * 4);
    }
}

void generate_swap_statement(CodeGenerator *generator, AstNode *node) {
    Token *var_a = node->data.swap_statement.var_a, *var_b = node->data.swap_statement.var_b;
    Symbol *sym_a, *sym_b;
    char *reg, *var_a_format, *var_b_format;
    sym_a = symbol_table_lookup(generator->symbol_table, var_a->value);
    sym_b = symbol_table_lookup(generator->symbol_table, var_b->value);
    reg = register_handler_request_register(generator->reg_handler, generator->fp,
                                            sym_a->value.var_symbol.var_size == BYTE ? AL : EAX);
    alsprintf(&var_a_format, "[%s]", get_var_name_formatted(sym_a->value.var_symbol.var_name));
    alsprintf(&var_b_format, "[%s]", get_var_name_formatted(sym_b->value.var_symbol.var_name));

    write_to_file(generator->fp, MOV, reg, var_a_format);
    write_to_file(generator->fp, XCHG, reg, var_b_format);
    write_to_file(generator->fp, MOV, var_a_format, reg);
    register_handler_free_register(generator->reg_handler, generator->fp, reg);
}
