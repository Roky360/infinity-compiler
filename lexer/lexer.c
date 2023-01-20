#include "lexer.h"
#include "../config/globals.h"
#include "../logging/logging.h"
#include "../io/io.h"
#include "../config/table_initializers.h"
#include "../config/constants.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Lexer *init_lexer(char *src) {
    int *first_line_offset;
    Lexer *lexer = malloc(sizeof(Lexer));
    if (!lexer)
        throw_memory_allocation_error(LEXER);

    lexer->src = src;
    lexer->src_len = strlen(src);
    lexer->idx = 0;
    lexer->row = 0;
    lexer->col = 0;
    lexer->c = src[0];

    lexer->line_offsets = init_list(sizeof(int *));
    first_line_offset = malloc(sizeof(int));
    *first_line_offset = 0;
    list_push(lexer->line_offsets, first_line_offset);

    return lexer;
}

void lexer_dispose(Lexer *lexer) {
    list_dispose(lexer->line_offsets);
    free(lexer);
}

void lexer_forward(Lexer *lexer) {
    unsigned int *line_offset;
    if (lexer->c == '\n') {
        (lexer->row)++;
        lexer->col = -1;
        line_offset = malloc(sizeof(unsigned int));
        *line_offset = lexer->idx + 1;
        list_push(lexer->line_offsets, line_offset);
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
    unsigned int token_start = lexer->col;
    void *lookup_res;
    int val_len = 1;
    char *val = calloc(val_len, sizeof(char));
    if (!val)
        throw_memory_allocation_error(LEXER);

    while (isalnum(lexer->c) || lexer->c == '_') {
        val = realloc(val, ++val_len);
        val[val_len - 2] = lexer->c;
        lexer_forward(lexer);
    }
    val[val_len - 1] = 0;

    lookup_res = hash_table_lookup(id_to_keyword_map, val);
    if (lookup_res) { // if this is a keyword
        if (!strcmp(val, VALUE_TRUE_KEYWORD))
            val = "1";
        else if (!strcmp(val, VALUE_FALSE_KEYWORD))
            val = "0";
        return init_token(val, (TokenType) atoi(lookup_res), lexer->row, token_start, val_len - 1);
    } else {
        return init_token(val, ID, lexer->row, token_start, val_len - 1);
    }
}

Token *lexer_parse_number_token(Lexer *lexer) {
    unsigned int token_start = lexer->col;
    int val_len;
    double res;
    char *val, *conversion_res;
    if (lexer->c == '-') { // if negative number
        val = strdup("-"); // -.2
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
        val[val_len - 2] = lexer->c;
        lexer_forward(lexer);
    }
    val[val_len - 1] = 0;

    res = strtod(val, &conversion_res);
    if (*conversion_res != '\0') {
        throw_exception_with_trace(LEXER, lexer, "Illegal number");
    }

    return (res == (int) res)
           ? init_token(val, INT, lexer->row, token_start, val_len - 1)
           : init_token(val, DOUBLE, lexer->row, token_start, val_len - 1);
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
    char *curr_char;
    void *lookup_res;
    Token *t;

    lexer_skip_whitespace(lexer);

    alsprintf(&curr_char, "%c", lexer->c);

    lookup_res = hash_table_lookup(char_to_token_type_map, curr_char);
    if (lookup_res) { // one-char tokens
        t = init_token(curr_char, (TokenType) atoi((char *) lookup_res), lexer->row, lexer->col, 1);
        lexer_forward(lexer);
        return t;
    }
    lookup_res = hash_table_lookup(char_to_to_lexing_function_map, curr_char);
    if (lookup_res) { // call token parser function
        t = ((Token *(*)(Lexer *, char *)) lookup_res)(lexer, curr_char);
        return t;
    } else {
        new_exception_with_trace(LEXER, lexer, lexer->row, lexer->col, 1, "Unknown token '%c'", lexer->c);
        return NULL;
    }
}
