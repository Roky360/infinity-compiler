#include "code_generator.h"
#include "../io/io.h"
#include "../logging/logging.h"
#include "instruction_generators.h"
#include "../config/table_initializers.h"
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
    generate_code_segment(generator);

    fclose(generator->fp);
}

void generate_data_segment(CodeGenerator *generator) {
    int i;
    char *var_type, *format;
    Symbol *symbol;

    write_to_file(generator->fp, "segment .data\n");

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
    write_to_file(generator->fp, "\nsection .text\n");
    write_to_file(generator->fp, LABEL_DEF, ENTRY_POINT_NAME);

    // call main function
    write_to_file(generator->fp, "\tcall " PROC_FORMAT "\n",
                  generator->starting_point->data.function_definition.func_name);
    write_to_file(generator->fp, "\n");

    // exit
    write_to_file(generator->fp, MOV, EAX, "1");
    write_to_file(generator->fp, XOR, EBX, EBX);
    write_to_file(generator->fp, SYSCALL_80H);
    write_to_file(generator->fp, "\n");

    // generate functions
    generate_block(generator, generator->root->data.compound.children);
}

void code_generator_apply_assignment(CodeGenerator *generator, DataType var_type, char *var_name, char *reg) {
    char *inst_format, *ref_format;
    alsprintf(&ref_format, "[%s]", var_name);

    switch (var_type) {
        case TYPE_BOOL:
        case TYPE_CHAR:
            write_to_file(generator->fp, MOV, ref_format, alsprintf(&inst_format, "byte %s", reg));
            free(inst_format);
            break;
        case TYPE_STRING:
            write_to_file(generator->fp, LEA, ref_format, reg); // pass the address of the string
            break;
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
    int i;
    for (i = 0; i < block->size; i++) {
        generate_statement(generator, (AstNode *) block->items[i]);
        if (((AstNode *) block->items[i])->type == AST_RETURN_STATEMENT)
            return 1;
    }
    return 0;
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

// calculates complicated expressions that contains variables and stores the result in ESI
void generate_arithmetic_expression(CodeGenerator *generator, Expression *expr) {
    char *esi, *expr_value_str;
    esi = register_handler_request_register(generator->reg_handler, generator->fp, ESI);

    if (expr->contains_variables) {

    } else {
        write_to_file(generator->fp, MOV, esi, alsprintf(&expr_value_str, "%d", (int) expr->value->value.double_value));
    }
    register_handler_free_register(generator->reg_handler, generator->fp, esi);
}

void generate_variable_declaration(CodeGenerator *generator, AstNode *node) {
    char *esi, *var_name;
    var_name = get_var_name_formatted(node->data.variable_declaration.var->name);

    generate_arithmetic_expression(generator, &node->data.variable_declaration.value->data.expression);
    esi = register_handler_request_register(generator->reg_handler, generator->fp, ESI);
    code_generator_apply_assignment(generator, node->data.variable_declaration.var->value->type, var_name, esi);

    write_to_file(generator->fp, "\n");
}

void generate_assignment(CodeGenerator *generator, AstNode *node) {
    char *esi, *var_name;
    Symbol *target_var;
    var_name = get_var_name_formatted(node->data.assignment.dst_variable->value);
    target_var = (Symbol *) hash_table_lookup(generator->symbol_table->table,
                                              node->data.assignment.dst_variable->value);

    generate_arithmetic_expression(generator, &node->data.assignment.expression->data.expression);
    esi = register_handler_request_register(generator->reg_handler, generator->fp, ESI);
    code_generator_apply_assignment(generator, target_var->value.var_symbol.type, var_name, esi);

    write_to_file(generator->fp, "\n");
}

void generate_function(CodeGenerator *generator, AstNode *node) {
    int returned;
    char *proc_name = get_proc_name_formatted(node->data.function_definition.func_name);

    // function name label
    write_to_file(generator->fp, GLOBAL, proc_name);
    write_to_file(generator->fp, LABEL_DEF, proc_name);
    // if function accepts arguments, use EBP
    if (node->data.function_definition.args->size > 0) {
        write_to_file(generator->fp, PUSH, EBP);
        write_to_file(generator->fp, MOV, EBP, ESP);
    }
    // generate body
    returned = generate_block(generator, node->data.function_definition.body);

    if (!returned) {
        write_to_file(generator->fp, "\tret\n");
    }
    write_to_file(generator->fp, COMMENT, proc_name);

    write_to_file(generator->fp, "\n");
    free(proc_name);
}

void generate_function_call(CodeGenerator *generator, AstNode *node) {}

void generate_if_statement(CodeGenerator *generator, AstNode *node) {
    char *esi;
    char *false_label = generate_label();

    generate_arithmetic_expression(generator, &node->data.if_statement.condition->data.expression);
    // ESI holds the result of the expression
    esi = register_handler_request_register(generator->reg_handler, generator->fp, ESI);

    write_to_file(generator->fp, CMP, esi, 0);
    write_to_file(generator->fp, JE, false_label);
    register_handler_free_register(generator->reg_handler, generator->fp, esi); // free ESI

    // generate body
    generate_block(generator, node->data.if_statement.body_node);
    // else block
    write_to_file(generator->fp, LABEL_DEF, false_label);
    generate_block(generator, node->data.if_statement.else_node);

    write_to_file(generator->fp, "\n");
    free(false_label);
}

void generate_simple_loop(CodeGenerator *generator, AstNode *node) {
    char *ecx;
    char *loop_count, *loop_label;
    ecx = register_handler_request_register(generator->reg_handler, generator->fp, ECX);
    loop_label = generate_label();

    write_to_file(generator->fp, MOV, ecx, alsprintf(&loop_count, "%d", node->data.loop.end));
    write_to_file(generator->fp, LABEL_DEF, loop_label);

    // generate loop body
    generate_block(generator, node->data.loop.body);

    write_to_file(generator->fp, LOOP, loop_label);
    write_to_file(generator->fp, "\n");

    register_handler_free_register(generator->reg_handler, generator->fp, ecx);
}

void generate_loop_with_counter(CodeGenerator *generator, AstNode *node) {}

void generate_loop(CodeGenerator *generator, AstNode *node) {
    node->data.loop.loop_counter_name
    ? generate_loop_with_counter(generator, node)
    : generate_simple_loop(generator, node);
}

void generate_return_statement(CodeGenerator *generator, AstNode *node) {
    char *esi;
    unsigned int arg_count = node->data.return_statement.parent_function_arg_count;

    if (node->data.return_statement.value_expr->data.expression.value->type != TYPE_VOID) {
        generate_arithmetic_expression(generator, &node->data.return_statement.value_expr->data.expression);
        // ESI holds the result of the expression
        esi = register_handler_request_register(generator->reg_handler, generator->fp, ESI);

        // end function and return
        write_to_file(generator->fp, MOV, EAX, esi);
        register_handler_free_register(generator->reg_handler, generator->fp, esi);
    }

    if (arg_count == 0) {
        // no args
        write_to_file(generator->fp, "\tret\n");
    } else {
        write_to_file(generator->fp, POP, EBP);
        write_to_file(generator->fp, RET, arg_count * 4);
    }
}
