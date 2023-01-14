#include "logging.h"
#include <stdio.h>
#include <stdlib.h>

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

void throw_memory_allocation_error(Caller caller) {
    log_error(caller, "Can't allocate memory.");
}
