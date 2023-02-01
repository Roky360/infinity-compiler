#ifndef INFINITY_COMPILER_LOGGING_H
#define INFINITY_COMPILER_LOGGING_H

#include "../lexer/lexer.h"

typedef enum Caller {
    COMPILER,
    LEXER,
    PARSER,
    SEMANTIC_ANALYZER,
    CODE_GENERATOR,
} Caller;

char *caller_type_to_str(Caller caller);

void log_curr_line(const Lexer *lexer);

void log_debug(Caller caller, const char *format, ...);

void log_error(Caller caller, const char *msg);

void log_warning(const Lexer *lexer, const char *msg);

void new_log_curr_line(const Lexer *lexer, unsigned int line, unsigned int col, int mark_length);

void message_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length, char *msg, ...);

void new_exception_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length, char *msg, ...);

void throw_memory_allocation_error(Caller caller);

void log_raw_debug(Caller caller, const char *color, const char *format, ...);

void set_unicode_mode();

void set_ascii_mode();

void print_unicode(const wchar_t *color, const wchar_t *format);

#endif //INFINITY_COMPILER_LOGGING_H
