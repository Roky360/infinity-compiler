#include "token_parsers.h"
#include "../logging/logging.h"
#include "ctype.h"

Token *lexer_parse_equal_char(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    if (lexer_peek(lexer, 1) == '=') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("==", EQUALS, lexer->row, token_start, 2);
    } else if (lexer_peek(lexer, 1) == '>') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("=>", THICK_ARROW, lexer->row, token_start, 2);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, ASSIGNMENT, lexer->row, token_start, 1);
    }
}

Token *lexer_parse_greater_than_char(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    if (lexer_peek(lexer, 1) == '=') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token(">=", GRATER_EQUAL, lexer->row, token_start, 2);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, GRATER_THAN, lexer->row, token_start, 1);
    }
}

Token *lexer_parse_lower_than_char(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    if (lexer_peek(lexer, 1) == '=') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("<=", LOWER_EQUAL, lexer->row, token_start, 2);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, LOWER_THAN, lexer->row, token_start, 1);
    }
}

Token *lexer_parse_slash_char(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    char peek = lexer_peek(lexer, 1);
    if (peek == '/') {
        lexer_skip_one_line_comment(lexer);
    } else if (peek == '-') {
        lexer_skip_multi_line_comment(lexer);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, DIVIDE_OP, lexer->row, token_start, 1);
    }
    return lexer_next_token(lexer);
}

Token *lexer_parse_string_token(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    int str_len = 1;
    char *val = calloc(str_len, sizeof(char));
    if (!val)
        throw_memory_allocation_error(LEXER);

    lexer_forward(lexer);
    while (lexer->c != '"') {
        val = realloc(val, ++str_len);
        val[str_len - 2] = get_escape_character(lexer);
        lexer_forward(lexer);
    }
    val[str_len - 1] = 0; // terminate string with '\0'
    lexer_forward(lexer);

    return init_token(val, STRING, lexer->row, token_start, str_len);
}

Token *lexer_parse_char_token(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    char *val = calloc(2, sizeof(char));
    if (!val)
        throw_memory_allocation_error(LEXER);

    lexer_forward(lexer);
    val[0] = get_escape_character(lexer);
    lexer_forward(lexer);
    if (lexer->c != '\'') {
        log_exception_with_trace(LEXER, lexer, lexer->row, lexer->col - 2, 3,
                                 "Char literal should contain only one character");
    }
    lexer_forward(lexer);

    return init_token(val, CHAR, lexer->row, token_start, 1);
}

Token *lexer_parse_minus_char(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    char peek = lexer_peek(lexer, 1);
    if (peek == '>') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("->", ARROW, lexer->row, token_start, 2);
    } else if (peek == '-') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("--", DEC, lexer->row, token_start, 2);
    } else if (peek == '.' || isdigit(peek)) {
        return lexer_parse_number_token(lexer);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, SUB_OP, lexer->row, lexer->col, 1);
    }
}

Token *lexer_parse_plus_char(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    if (lexer_peek(lexer, 1) == '+') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("++", INC, lexer->row, token_start, 2);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, ADD_OP, lexer->row, token_start, 1);
    }
}

Token *lexer_parse_exclamation_char(Lexer *lexer, char *curr_char) {
    unsigned int token_start = lexer->col;
    if (lexer_peek(lexer, 1) == '=') {
        lexer_forward(lexer);
        lexer_forward(lexer);
        return init_token("!=", NOT_EQUAL, lexer->row, token_start, 2);
    } else {
        lexer_forward(lexer);
        return init_token(curr_char, FACTORIAL_OP, lexer->row, token_start, 1);
    }
}

/*****/
char get_escape_character(Lexer *lexer) {
    if (lexer->c == '\\') {
        lexer_forward(lexer);
        switch (lexer->c) {
            case 'n':
                return '\n';
            case 't':
                return '\t';
            case '\\':
                return '\\';
            case '"':
                return '"';
            case '\'':
                return '\'';
            default:
                return lexer->c;
        }
    }
    return lexer->c;
}
