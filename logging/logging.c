#include "logging.h"
#include "../io/io.h"
#include "../config/console_colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

char *caller_type_to_str(Caller caller) {
    switch (caller) {
        case LEXER:
            return "Lexer";
        case PARSER:
            return "Parser";
        case COMPILER:
            return "Compiler";
        case SEMANTIC_ANALYZER:
            return "Analyzer";
        case CODE_GENERATOR:
            return "Code Generator";
        default:
            return "Unknown";
    }
}

void log_curr_line(const Lexer *lexer) {
    unsigned int rowNoLen, i;

    // print line number
    rowNoLen = printf(" %d", lexer->row + 1);
    printf(" |  ");
    // print source code line
    i = lexer->idx - lexer->col;
    while (lexer->src[i] != '\n' && lexer->src[i] != 0)
        printf("%c", lexer->src[i++]);
    // print message
    printf("\n%*s |  %*s^\n", rowNoLen, "", lexer->col, "");
}

void log_debug(Caller caller, const char *msg) {
    printf("[%s] %s\n\n", caller_type_to_str(caller), msg);
}

void log_error(Caller caller, const char *msg) {
    log_debug(caller, msg);
    exit(1);
}

void log_warning(const Lexer *lexer, const char *msg) {
    log_curr_line(lexer);
    printf("[Warning] %s\n", msg);
}

void throw_exception_with_trace(Caller caller, const Lexer *lexer, const char *msg) {
    log_curr_line(lexer);
    log_debug(caller, msg);

    exit(1);
}

void new_log_curr_line(const Lexer *lexer, unsigned int line, unsigned int col, int mark_length) {
    unsigned int line_no_len, i = 0;
    int start_line = *(int *) lexer->line_offsets->items[line];

    line_no_len = printf(" %d", line);
    printf(" |  ");

    while (lexer->src[start_line + i] != '\n' && lexer->src[i] != 0) {
        if (i == col)
            // color
            printf(RED);
        putchar(lexer->src[start_line + i++]);
        if (i - col == mark_length)
            // reset color
            printf(RESET);
    }
    // print message
    printf("\n%*s |  %*s%s^%s\n", line_no_len, "", col, "", RED, RESET);
}

void new_exception_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                              char *msg, ...) {
    char *format;
    va_list args;
    va_start(args, msg);
    alsprintf(&format, "%s\n", msg); // add \n to the message

    // log message
    printf("[%s] ", caller_type_to_str(caller));
    vprintf(format, args);
    // log source code line
    new_log_curr_line(lexer, line, col, mark_length);

    free(format);
    va_end(args);
    exit(1);
}

void throw_memory_allocation_error(Caller caller) {
    log_error(caller, "Can't allocate memory.");
}
