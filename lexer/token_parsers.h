#ifndef INFINITY_COMPILER_TOKEN_PARSERS_H
#define INFINITY_COMPILER_TOKEN_PARSERS_H

#include "lexer.h"
#include "../token/token.h"

Token *lexer_parse_equal_char(Lexer *lexer, char *currC);

Token *lexer_parse_greater_than_char(Lexer *lexer, char *curr_char);

Token *lexer_parse_lower_than_char(Lexer *lexer, char *curr_char);

Token *lexer_parse_slash_char(Lexer *lexer, char *curr_char);

Token *lexer_parse_string_token(Lexer *lexer, char *curr_char);

Token *lexer_parse_char_token(Lexer *lexer, char *curr_char);

Token *lexer_parse_minus_char(Lexer *lexer, char *curr_char);

Token *lexer_parse_plus_char(Lexer *lexer, char *curr_char);

Token *lexer_parse_exclamation_char(Lexer *lexer, char *curr_char);

// ---
char get_escape_character(Lexer *lexer);

#endif //INFINITY_COMPILER_TOKEN_PARSERS_H
