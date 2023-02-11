#ifndef INFINITY_COMPILER_CODE_GENERATOR_H
#define INFINITY_COMPILER_CODE_GENERATOR_H

#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"
#include "register_handler.h"

#define EXPR_RES_REG EAX

/** Naming Conventions */
#define ENTRY_POINT_NAME "_start"
/** Helper Procedures */
#define INCLUDE_ASM_PATH "../config/include.asm"

#define POWER_PROC "Power"
#define FACT_PROC "Fact"
#define PRINT_PROC "Print"
#define PRINT_NEW_LINE_PROC "PrintNewLine"
#define PRINT_INT_PROC "PrintInt"

#define TRUE_STR_VAR "true_str"
#define FALSE_STR_VAR "false_str"

typedef struct CodeGenerator {
    SymbolTable *symbol_table;
    RegisterHandler *reg_handler;

    AstNode *root; // root node of the AST tree
    AstNode *starting_point; // starting point of the application

    char *target_path; // output file path
    FILE *fp; // target file pointer
} CodeGenerator;

CodeGenerator *
init_code_generator(SymbolTable *symbol_table, AstNode *root, AstNode *starting_point, char *target_path);

void dispose_code_generator(CodeGenerator *generator);

void code_generator_generate(CodeGenerator *generator);

void generate_data_segment(CodeGenerator *generator);

void generate_bss_segment(CodeGenerator *generator);

// generates the code segment and calls the starting point function
void generate_code_segment(CodeGenerator *generator);

char *get_variable_size_prefix(CodeGenerator *generator, char *var_name);

void code_generator_apply_assignment(CodeGenerator *generator, DataType var_type, char *var_name, char *reg);

int generate_block(CodeGenerator *generator, List *block);

void generate_statement(CodeGenerator *generator, AstNode *node);

void generate_complicated_arithmetic_expression(CodeGenerator *generator, List *postfix_expr_lst);

void generate_arithmetic_expression(CodeGenerator *generator, Expression *expr);

void generate_variable_declaration(CodeGenerator *generator, AstNode *node);

void generate_assignment(CodeGenerator *generator, AstNode *node);

void generate_function(CodeGenerator *generator, AstNode *node);

void generate_function_call(CodeGenerator *generator, AstNode *node);

void generate_if_statement(CodeGenerator *generator, AstNode *node);

void generate_simple_loop(CodeGenerator *generator, AstNode *node);

void generate_loop_with_counter(CodeGenerator *generator, AstNode *node);

void generate_loop(CodeGenerator *generator, AstNode *node);

void generate_while_loop(CodeGenerator *generator, AstNode *node);

void generate_return_statement(CodeGenerator *generator, AstNode *node);

void generate_swap_statement(CodeGenerator *generator, AstNode *node);

#endif //INFINITY_COMPILER_CODE_GENERATOR_H
