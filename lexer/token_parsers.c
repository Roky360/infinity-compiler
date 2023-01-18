#include "token_parsers.h"
#include "../logging/logging.h"
#include "ctype.h"

Token *lexer_parse_equal_char(Lexer *lexer, char *curr_char) {
    if (lexer_peek(lexer, 1) == '=') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("==", EQUALS);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, ASSIGNMENT);
    }
}

Token *lexer_parse_greater_than_char(Lexer *lexer, char *curr_char) {
    if (lexer_peek(lexer, 1) == '=') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token(">=", GRATER_EQUAL);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, GRATER_THAN);
    }
}

Token *lexer_parse_lower_than_char(Lexer *lexer, char *curr_char) {
    if (lexer_peek(lexer, 1) == '=') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("<=", LOWER_EQUAL);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, LOWER_THAN);
    }
}

Token *lexer_parse_slash_char(Lexer *lexer, char *curr_char) {
    char peek = lexer_peek(lexer, 1);
    if (peek == '/') {
        lexer_skip_one_line_comment(lexer);
    } else if (peek == '-') {
        lexer_skip_multi_line_comment(lexer);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, DIVIDE_OP);
    }
    return lexer_next_token(lexer);
}

Token *lexer_parse_string_token(Lexer *lexer, char *curr_char) {
    int str_len = 1;
    char *val = calloc(str_len, sizeof(char));
    if (!val)
        throw_memory_allocation_error(LEXER);

    lexer_forward(lexer);
    while (lexer->c != '"') {
        val = realloc(val, ++str_len);
        val[str_len - 2] = lexer->c;
        lexer_forward(lexer);
    }
    val[str_len - 1] = 0; // terminate string with '\0'

    return init_token(val, STRING);
}

Token *lexer_parse_char_token(Lexer *lexer, char *curr_char) {
    char *val = calloc(2, sizeof(char));
    if (!val)
        throw_memory_allocation_error(LEXER);

    lexer_forward(lexer);
    val[0] = lexer->c;
    lexer_forward(lexer);
    if (lexer->c != '\'') {
        throw_exception_with_trace(LEXER, lexer, "Char literal should contain only one character");
    }
    lexer_forward(lexer);

    return init_token(val, CHAR);
}

Token *lexer_parse_minus_char(Lexer *lexer, char *curr_char) {
    char peek = lexer_peek(lexer, 1);
    if (peek == '>') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("->", ARROW);
    } else if (peek == '-') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("--", DEC);
    } else if (peek == '.' || isdigit(peek)) {
        return lexer_parse_number_token(lexer);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, SUB_OP);
    }
}

Token *lexer_parse_plus_char(Lexer *lexer, char *curr_char) {
    if (lexer_peek(lexer, 1) == '+') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("++", INC);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, ADD_OP);
    }
}

Token *lexer_parse_exclamation_char(Lexer *lexer, char *curr_char) {
    if (lexer_peek(lexer, 1) == '=') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("!=", NOT_EQUAL);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, FACTORIAL_OP);
    }
}
