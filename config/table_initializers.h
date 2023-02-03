#ifndef INFINITY_COMPILER_TABLE_INITIALIZERS_H
#define INFINITY_COMPILER_TABLE_INITIALIZERS_H

#include "globals.h"

void dispose_string(void *item);

/* Semantic Analyzer routing map */
extern HashTable *ast_type_to_analyzer_map;

void init_ast_type_to_analyzer_table();

/* Parser routing map */
extern HashTable *statement_to_parser_map;

void init_statement_to_parser_table();

/* Lexer Maps */
// maps a character to a function that parses the tokens starting with that character
extern HashTable *char_to_to_lexing_function_map;

void init_char_to_lexing_function_map();

// maps a character to the token type that associated with it. works for tokens that are one character long
extern HashTable *char_to_token_type_map;

void init_char_to_token_type_map();

// maps an value represents a keyword to a TokenType associated with it
extern HashTable *id_to_keyword_map;

void init_id_to_keyword_map();

/* Expression evaluator */
extern HashTable *operator_to_applier_function_map;

void init_operator_to_applier_function_map();

extern HashTable *precedence_map;

void init_precedence_map();

/* Code Generator */
extern HashTable *statement_to_generator_map;

void init_statement_to_generator_map();

extern HashTable *operator_to_generator_map;

void init_operator_to_generator_map();

#endif //INFINITY_COMPILER_TABLE_INITIALIZERS_H
