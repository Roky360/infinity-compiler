#ifndef INFINITY_COMPILER_LOGGING_H
#define INFINITY_COMPILER_LOGGING_H

#include "../lexer/lexer.h"

typedef enum {
    COMPILER,
    LEXER,
    PARSER,
    SEMANTIC_ANALYZER,
    CODE_GENERATOR,
} Caller;

char *caller_type_to_str(Caller caller);

void log_curr_line(const Lexer *lexer);

void log_debug(Caller caller, const char *msg);

void log_error(Caller caller, const char *msg);

void log_warning(const Lexer *lexer, const char *msg);

void throw_exception_with_trace(Caller caller, const Lexer *lexer, const char *msg);

void throw_memory_allocation_error(Caller caller);

#endif //INFINITY_COMPILER_LOGGING_H
