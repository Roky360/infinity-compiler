#include "table_initializers.h"
#include "../ast/ast.h"
#include "../semantic_analyzer/semantic_analyzer.h"
#include "../lexer/token_parsers.h"
#include "../io/io.h"
#include "../parser/parser.h"
#include "../config/constants.h"
#include "../expression_evaluator/expression_evaluator.h"
#include "../expression_evaluator/operator_appliers.h"
#include "../code_generator/code_generator.h"
#include <stdlib.h>
#include <string.h>

void dispose_string(void *item) {
    free((char *) item);
}

HashTable *ast_type_to_analyzer_map;

void init_ast_type_to_analyzer_table() {
    int i;
    char *id_ptr;
    AstType keys[] = {
            AST_START_EXPRESSION,
            AST_VARIABLE_DECLARATION,
            AST_ASSIGNMENT,
            AST_FUNCTION_DEFINITION,
//            AST_FUNCTION_CALL,
            AST_IF_STATEMENT,
            AST_LOOP,
            AST_RETURN_STATEMENT,
    };
    void (*values[])(SemanticAnalyzer *, AstNode *, AstNode *) = {
            semantic_analyze_start_statement,
            semantic_analyze_variable_declaration,
            semantic_analyze_assignment,
            semantic_analyze_function,
//            NULL,
            semantic_analyze_if_statement,
            semantic_analyze_loop_statement,
            semantic_analyze_return_statement,
    };
    ast_type_to_analyzer_map = init_hash_table(19, NULL); // no need to free the functions...

    for (i = 0; i < ARRLEN(keys); i++) {
        hash_table_insert(
                ast_type_to_analyzer_map,
                alsprintf(&id_ptr, "%d", keys[i]),
                values[i]
        );
    }
}

HashTable *statement_to_parser_map;

void init_statement_to_parser_table() {
    int i;
    char *id_ptr;
    TokenType keys[] = {
            START_KEYWORD,
            ID,
            FUNC_KEYWORD,
            INT_KEYWORD,
            CHAR_KEYWORD,
            BOOL_KEYWORD,
            STRING_KEYWORD,
            IF_KEYWORD,
            LOOP_KEYWORD,
            RETURN_KEYWORD,
    };
    AstNode *(*values[])(Parser *) = {
            parser_parse_start_expression,
            parser_parse_id,
            parser_parse_function_definition,
            parser_parse_var_declaration,
            parser_parse_var_declaration,
            parser_parse_var_declaration,
            parser_parse_var_declaration,
            parser_parse_if_statement,
            parser_parse_loop,
            parser_parse_return_statement,
    };
    statement_to_parser_map = init_hash_table(19, NULL); // no need to free the functions...

    for (i = 0; i < ARRLEN(keys); i++) {
        hash_table_insert(
                statement_to_parser_map,
                alsprintf(&id_ptr, "%d", keys[i]),
                values[i]
        );
    }
}

HashTable *char_to_to_lexing_function_map;

void init_char_to_lexing_function_map() {
    int i;
    char *id_ptr;
    char keys[] = {
            '=',
            '>',
            '<',
            '/',
            '"',
            '\'',
            '-',
            '+',
            '!',
    };
    Token *(*values[])(Lexer *, char *) = {
            lexer_parse_equal_char,
            lexer_parse_greater_than_char,
            lexer_parse_lower_than_char,
            lexer_parse_slash_char,
            lexer_parse_string_token,
            lexer_parse_char_token,
            lexer_parse_minus_char,
            lexer_parse_plus_char,
            lexer_parse_exclamation_char,
    };
    char_to_to_lexing_function_map = init_hash_table(97, NULL); // no need to free the functions...

    for (i = 0; i < ARRLEN(keys); i++) {
        hash_table_insert(
                char_to_to_lexing_function_map,
                alsprintf(&id_ptr, "%c", keys[i]),
                values[i]
        );
    }

    /* add numbers for parse_int_token */
    for (i = 0; i <= 9; i++) {
        hash_table_insert(
                char_to_to_lexing_function_map,
                alsprintf(&id_ptr, "%d", i),
                lexer_parse_number_token
        );
    }
    // floating point
    hash_table_insert(
            char_to_to_lexing_function_map,
            strdup("."),
            lexer_parse_number_token
    );

    /* add letters for parse_id_token */
    for (i = 'A'; i <= 'Z'; i++) {
        hash_table_insert(
                char_to_to_lexing_function_map,
                alsprintf(&id_ptr, "%c", i), // add capital letter
                lexer_parse_id_token
        );
        hash_table_insert(
                char_to_to_lexing_function_map,
                alsprintf(&id_ptr, "%c", i + ('a' - 'A')), // small letter
                lexer_parse_id_token
        );
    }
    // underscore
    hash_table_insert(
            char_to_to_lexing_function_map,
            strdup("_"),
            lexer_parse_id_token
    );
}

HashTable *char_to_token_type_map;

void init_char_to_token_type_map() {
    int i;
    char *id_ptr;
    char types[] = {
            '(',
            ')',
            '{',
            '}',
            '[',
            ']',
            ';',
            ',',
            ':',
            '*',
            '%',
            '^',
            '\0',
    };
    TokenType values[] = {
            L_PARENTHESES,
            R_PARENTHESES,
            L_CURLY_BRACE,
            R_CURLY_BRACE,
            L_SQUARE_BRACKET,
            R_SQUARE_BRACKET,
            SEMICOLON,
            COMMA,
            COLON,
            MUL_OP,
            MODULUS_OP,
            POWER_OP,
            EOF_TOKEN,
    };
    char_to_token_type_map = init_hash_table(23, dispose_string); // no need to free a function
    // initialize all fields with the init_token function
    for (i = 0; i < ARRLEN(types); i++) {
        hash_table_insert(
                char_to_token_type_map,
                alsprintf(&id_ptr, "%c", types[i]),
                alsprintf(&id_ptr, "%d", values[i])
        );
    }
}

HashTable *id_to_keyword_map;

void init_id_to_keyword_map() {
    int i;
    char *id_ptr;
    char *ids[] = { // keys
            VALUE_VOID_KEYWORD,
            VALUE_INT_KEYWORD,
            VALUE_STRING_KEYWORD,
            VALUE_BOOL_KEYWORD,
            VALUE_CHAR_KEYWORD,
            VALUE_RETURN_KEYWORD,
            VALUE_FUNC_KEYWORD,
            VALUE_IF_KEYWORD,
            VALUE_ELSE_KEYWORD,
            VALUE_LOGICAL_AND_KEYWORD,
            VALUE_LOGICAL_OR_KEYWORD,
            VALUE_NOT_KEYWORD,
            VALUE_LOOP_KEYWORD,
            VALUE_TO_KEYWORD,
            VALUE_TIMES_KEYWORD,
            VALUE_START_KEYWORD,
            VALUE_TRUE_KEYWORD,
            VALUE_FALSE_KEYWORD,
    };
    TokenType types[] = {
            VOID_KEYWORD,
            INT_KEYWORD,
            STRING_KEYWORD,
            BOOL_KEYWORD,
            CHAR_KEYWORD,
            RETURN_KEYWORD,
            FUNC_KEYWORD,
            IF_KEYWORD,
            ELSE_KEYWORD,
            AND_OPERATOR_KEYWORD,
            OR_OPERATOR_KEYWORD,
            NOT_OPERATOR_KEYWORD,
            LOOP_KEYWORD,
            TO_KEYWORD,
            TIMES_KEYWORD,
            START_KEYWORD,
            INT,
            INT,
    };
    id_to_keyword_map = init_hash_table(31, dispose_string);

    for (i = 0; i < ARRLEN(types); i++) {
        hash_table_insert(
                id_to_keyword_map,
                strdup(ids[i]),
                alsprintf(&id_ptr, "%d", types[i])
        );
    }
}

/* Expression evaluator */
HashTable *operator_to_applier_function_map;

void init_operator_to_applier_function_map() {
    int i;
    char *ops[] = { // keys
            OP_ADD,
            OP_SUB,
            OP_MUL,
            OP_DIV,
            OP_POW,
            OP_MOD,
            OP_FACT,
            OP_LOGICAL_AND,
            OP_LOGICAL_OR,
            OP_NOT,
            OP_EQUALITY,
            OP_NOT_EQUAL,
            OP_GRATER_THAN,
            OP_GRATER_EQUAL,
            OP_LOWER_THAN,
            OP_LOWER_EQUAL,
    };
    double (*applier_func[])(double, double, char *, char *) = {
            apply_addition,
            apply_subtraction,
            apply_multiplication,
            apply_division,
            apply_power,
            apply_modulus,
            apply_factorial,
            apply_logical_and,
            apply_logical_or,
            apply_not,
            apply_equality,
            apply_not_equal,
            apply_greater_than,
            apply_greater_equal,
            apply_lower_than,
            apply_lower_equal,
    };
    operator_to_applier_function_map = init_hash_table(31, NULL); // no need to free the functions...

    for (i = 0; i < ARRLEN(applier_func); i++) {
        hash_table_insert(
                operator_to_applier_function_map,
                strdup(ops[i]),
                applier_func[i]
        );
    }
}

HashTable *precedence_map;

void init_precedence_map() {
    int i;
    char *ops[] = { // keys
            // 8
            OP_POW,
            OP_FACT,
            // 7
            OP_NOT,
            // 6
            OP_MUL,
            OP_DIV,
            OP_MOD,
            // 5
            OP_ADD,
            OP_SUB,
            // 4
            OP_GRATER_THAN,
            OP_GRATER_EQUAL,
            OP_LOWER_THAN,
            OP_LOWER_EQUAL,
            // 3
            OP_EQUALITY,
            OP_NOT_EQUAL,
            // 2
            OP_LOGICAL_AND,
            // 1
            OP_LOGICAL_OR,

    };
    char *precedences[] = {
            "8",
            "8",
            "7",
            "6",
            "6",
            "6",
            "5",
            "5",
            "4",
            "4",
            "4",
            "4",
            "3",
            "3",
            "2",
            "1",
    };
    precedence_map = init_hash_table(31, dispose_string);

    for (i = 0; i < ARRLEN(ops); i++) {
        hash_table_insert(
                precedence_map,
                strdup(ops[i]),
                strdup(precedences[i])
        );
    }
}

/* Code Generator */
HashTable *statement_to_generator_map;

void init_statement_to_generator_map() {
    int i;
    char *id_ptr;
    AstType ast_types[] = { // keys
            AST_VARIABLE_DECLARATION,
            AST_ASSIGNMENT,
            AST_FUNCTION_DEFINITION,
            AST_FUNCTION_CALL,
            AST_IF_STATEMENT,
            AST_LOOP,
            AST_RETURN_STATEMENT,
    };
    void (*generator_funcs[])(CodeGenerator *, AstNode *) = { // values
            generate_variable_declaration,
            generate_assignment,
            generate_function,
            generate_function_call,
            generate_if_statement,
            generate_loop,
            generate_return_statement,
    };
    statement_to_generator_map = init_hash_table(19, NULL);

    for (i = 0; i < ARRLEN(ast_types); i++) {
        hash_table_insert(
                statement_to_generator_map,
                alsprintf(&id_ptr, "%d", ast_types[i]),
                generator_funcs[i]
        );
    }
}
