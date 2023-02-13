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

typedef enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    SUCCESS,
} LogLevel;

char *caller_type_to_str(Caller caller);

char *log_level_to_str(LogLevel level);

char *get_log_level_color(LogLevel level);

void new_log_curr_line(const Lexer *lexer, const char *color, unsigned int line, unsigned int col, int mark_length);

void log_success(Caller caller, const char *format, ...);

void log_debug(Caller caller, const char *format, ...);

void log_info(Caller caller, const char *format, ...);

void log_warning(Caller caller, const char *format, ...);

void log_error(Caller caller, const char *format, ...);

void log_error_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                              const char *format, ...);

void log_exception_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                              const char *format, ...);

void log_warning_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                            const char *format, ...);

void throw_memory_allocation_error(Caller caller);

/** Unicode */
void log_raw_debug(Caller caller, const char *color, const char *format, ...);

void set_unicode_mode();

void set_ascii_mode();

void print_unicode(const wchar_t *color, const wchar_t *format);

#endif //INFINITY_COMPILER_LOGGING_H
