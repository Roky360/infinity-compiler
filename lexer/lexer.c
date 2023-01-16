#include "lexer.h"
#include "../config/globals.h"
#include "../logging/logging.h"
#include "../io/io.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Lexer *init_lexer(char *src) {
    Lexer *lexer = malloc(sizeof(Lexer));
    if (!lexer)
        throw_memory_allocation_error(LEXER);

    lexer->src = src;
    lexer->src_len = strlen(src);
    lexer->idx = 0;
    lexer->row = 0;
    lexer->col = 0;
    lexer->c = src[0];

    return lexer;
}

void lexer_dispose(Lexer *lexer) {
    free(lexer);
}

void lexer_forward(Lexer *lexer) {
    if (lexer->c == '\n') {
        (lexer->row)++;
        lexer->col = -1;
    }
    (lexer->idx)++;
    (lexer->col)++;
    lexer->c = lexer->src[lexer->idx];
}

char lexer_peek(Lexer *lexer, int amount) {
    // clamp amount between 0 and src_len
    return lexer->src[MAX(0, MIN(lexer->src_len, lexer->idx + amount))];
}

void lexer_skip_whitespace(Lexer *lexer) {
    while (lexer->c == ' ' || lexer->c == '\t' || lexer->c == '\n') {
        lexer_forward(lexer);
    }
}

Token *lexer_parse_id_token(Lexer *lexer) {
    char *val = calloc(1, sizeof(char));
    if (!val)
        throw_memory_allocation_error(LEXER);

    while (isalnum(lexer->c) || lexer->c == '_') {
        val = realloc(val, (strlen(val) + 2) * sizeof(char));
        strcat(val, (char[]) {lexer->c, 0});
        lexer_forward(lexer);
    }

    if (!strcmp(val, "void"))
        return init_token(val, VOID_KEYWORD);
    else if (!strcmp(val, "int"))
        return init_token(val, INT_KEYWORD);
    else if (!strcmp(val, "string"))
        return init_token(val, STRING_KEYWORD);
    else if (!strcmp(val, "bool"))
        return init_token(val, BOOL_KEYWORD);
    else if (!strcmp(val, "char"))
        return init_token(val, CHAR_KEYWORD);
    else if (!strcmp(val, "return"))
        return init_token(val, RETURN_KEYWORD);
    else if (!strcmp(val, "func"))
        return init_token(val, FUNC_KEYWORD);
    else if (!strcmp(val, "if"))
        return init_token(val, IF_KEYWORD);
    else if (!strcmp(val, "else"))
        return init_token(val, ELSE_KEYWORD);
    else if (!strcmp(val, "and"))
        return init_token(val, AND_OPERATOR_KEYWORD);
    else if (!strcmp(val, "or"))
        return init_token(val, OR_OPERATOR_KEYWORD);
    else if (!strcmp(val, "not"))
        return init_token(val, NOT_OPERATOR_KEYWORD);
    else if (!strcmp(val, "loop"))
        return init_token(val, LOOP_KEYWORD);
    else if (!strcmp(val, "to"))
        return init_token(val, TO_KEYWORD);
    else if (!strcmp(val, "times"))
        return init_token(val, TIMES_KEYWORD);
    else if (!strcmp(val, "start"))
        return init_token(val, START_KEYWORD);
    else if (!strcmp(val, "true"))
        return init_token("1", INT);
    else if (!strcmp(val, "false"))
        return init_token("0", INT);

    return init_token(val, ID);
}

Token *lexer_parse_int_token(Lexer *lexer) {
    int floating_point_met = 0, val_len;
    char *val;
    if (lexer->c == '-') { // if negative number
        val = strdup("-");
        val_len = 2;
        lexer_forward(lexer);
    } else {
        val = strdup("");
        val_len = 1;
    }
    if (!val)
        throw_memory_allocation_error(LEXER);

    while (isdigit(lexer->c) || lexer->c == '.') {
        val = realloc(val, ++val_len);
        if (lexer->c == '.') {
            if (!floating_point_met) {
                floating_point_met = 1;
            } else {
                throw_exception_with_trace(LEXER, lexer, "More than one floating point in constant");
            }
        }
        strcat(val, (char[]) {lexer->c, 0});
        lexer_forward(lexer);
    }

    return floating_point_met ? init_token(val, DOUBLE) : init_token(val, INT);
}

Token *lexer_parse_string_token(Lexer *lexer) {
    char *val = calloc(1, sizeof(char));
    if (!val)
        throw_memory_allocation_error(LEXER);

    lexer_forward(lexer);
    while (lexer->c != '"') {
        val = realloc(val, (strlen(val) + 2) * sizeof(char));
        strcat(val, (char[]) {lexer->c, 0});
        lexer_forward(lexer);
    }

    return init_token(val, STRING);
}

Token *lexer_parse_char_token(Lexer *lexer) {
    char *val = calloc(2, sizeof(char));
    if (!val)
        throw_memory_allocation_error(LEXER);

    lexer_forward(lexer);
    val[0] = lexer->c;
    lexer_forward(lexer);
    if (lexer->c != '\'') {
        throw_exception_with_trace(LEXER, lexer, "Char literal should contain only one character");
    }

    return init_token(val, CHAR);
}

void lexer_skip_one_line_comment(Lexer *lexer) {
    while (lexer->c != '\n') {
        if (lexer->c == 0)
            break;
        lexer_forward(lexer);
    }
}

void lexer_skip_multi_line_comment(Lexer *lexer) {
    unsigned int row = lexer->row, col = lexer->col, idx = lexer->idx;
    lexer_forward(lexer);
    lexer_forward(lexer);
    while (!(lexer->c == '-' && lexer_peek(lexer, 1) == '/')) {
        if (lexer->c == 0) {
            lexer->row = row;
            lexer->col = col;
            lexer->idx = idx;
            throw_exception_with_trace(LEXER, lexer, "Comment unclosed at end of file");
        }
        lexer_forward(lexer);
    }
    lexer_forward(lexer);
    lexer_forward(lexer);
}

Token *lexer_next_token(Lexer *lexer) {
    Token *t;
    char *errorMsg;
    char *currC;

    lexer_skip_whitespace(lexer);

    if (isalpha(lexer->c) || lexer->c == '_')
        t = lexer_parse_id_token(lexer);
    else if (isdigit(lexer->c) || lexer->c == '.'
             || (lexer->c == '-' && (isdigit(lexer_peek(lexer, 1)) || lexer_peek(lexer, 1) == '.')))
        t = lexer_parse_int_token(lexer);
    else {
        alsprintf(&currC, "%c", lexer->c);

        switch (lexer->c) {
            case '(':
                t = init_token(currC, L_PARENTHESES);
                break;
            case ')':
                t = init_token(currC, R_PARENTHESES);
                break;
            case '{':
                t = init_token(currC, L_CURLY_BRACE);
                break;
            case '}':
                t = init_token(currC, R_CURLY_BRACE);
                break;
            case '[':
                t = init_token(currC, L_SQUARE_BRACKET);
                break;
            case ']':
                t = init_token(currC, R_SQUARE_BRACKET);
                break;
            case ';':
                t = init_token(currC, SEMICOLON);
                break;
            case ',':
                t = init_token(currC, COMMA);
                break;
            case ':':
                t = init_token(currC, COLON);
                break;
            case '=':
                if (lexer_peek(lexer, 1) == '=') {
                    t = init_token("==", EQUALS);
                    lexer_forward(lexer);
                } else
                    t = init_token(currC, ASSIGNMENT);
                break;
            case '>':
                if (lexer_peek(lexer, 1) == '=') {
                    t = init_token(">=", GRATER_EQUAL);
                    lexer_forward(lexer);
                } else
                    t = init_token(currC, GRATER_THAN);
                break;
            case '<':
                if (lexer_peek(lexer, 1) == '=') {
                    t = init_token("<=", LOWER_EQUAL);
                    lexer_forward(lexer);
                } else
                    t = init_token(currC, LOWER_THAN);
                break;
            case '/':
                if (lexer_peek(lexer, 1) == '/') {
                    lexer_skip_one_line_comment(lexer);
                } else if (lexer_peek(lexer, 1) == '-') {
                    lexer_skip_multi_line_comment(lexer);
                } else {
                    t = init_token(currC, DIVIDE_OP);
                    break;
                }
                return lexer_next_token(lexer);
            case '"':
                t = lexer_parse_string_token(lexer);
                break;
            case '\'':
                t = lexer_parse_char_token(lexer);
                break;
            case '-':
                if (lexer_peek(lexer, 1) == '>') {
                    t = init_token("->", ARROW);
                    lexer_forward(lexer);
                } else if (lexer_peek(lexer, 1) == '-') {
                    t = init_token("--", DEC);
                    lexer_forward(lexer);
                } else {
                    t = init_token(currC, SUB_OP);
                }
                break;
            case '+':
                if (lexer_peek(lexer, 1) == '+') {
                    t = init_token("++", INC);
                    lexer_forward(lexer);
                } else {
                    t = init_token(currC, ADD_OP);
                }
                break;
            case '*':
                t = init_token(currC, MUL_OP);
                break;
            case '%':
                t = init_token(currC, MODULUS_OP);
                break;
            case '^':
                t = init_token(currC, POWER_OP);
                break;
            case '!':
                if (lexer_peek(lexer, 1) == '=') {
                    t = init_token("!=", NOT_EQUAL);
                    lexer_forward(lexer);
                } else
                    t = init_token(currC, FACTORIAL_OP);
                break;
            case 0: // EOF
                t = init_token(currC, EOF_TOKEN);
                break;
            default:
                alsprintf(&errorMsg, "Unknown token '%c'", lexer->c);
                throw_exception_with_trace(LEXER, lexer, errorMsg);
                break;
        }
        lexer_forward(lexer);
    }

    return t;
}
