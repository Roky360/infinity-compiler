#include "logging.h"
#include "../io/io.h"
#include "../config/globals.h"
#include "../config/console_colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>

char *caller_type_to_str(Caller caller) {
    switch (caller) {
        case LEXER:
            return "Lexer";
        case PARSER:
            return "Parser";
        case COMPILER:
            return "Compiler";
        case IO:
            return "IO";
        case SEMANTIC_ANALYZER:
            return "Analyzer";
        case CODE_GENERATOR:
            return "Code Generator";
        default:
            return "Unknown";
    }
}

char *log_level_to_str(LogLevel level) {
    switch (level) {
        case DEBUG:
            return "debug";
        case INFO:
        case SUCCESS:
            return "info";
        case WARNING:
            return "warning";
        case ERROR:
            return "error";
    }
}

char *get_log_level_color(LogLevel level) {
    switch (level) {
        case DEBUG:
            return WHITE_B;
        case INFO:
            return CYAN_B;
        case WARNING:
            return YELLOW_B;
        case ERROR:
            return RED_B;
        case SUCCESS:
            return GREEN_B;
    }
}

void log_msg(const char *color, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *buf = alsprintf(&buf, "%s\n", format);
#ifdef INF_SHOW_COLORS
    printf("%s", color);
#endif
    vprintf(buf, args);
#ifdef INF_SHOW_COLORS
    printf("%s", RESET);
#endif
    free(buf);
    va_end(args);
}

void new_log_curr_line(const Lexer *lexer, const char *color, unsigned int line, unsigned int col, int mark_length) {
    unsigned int line_no_len, i = *(int *) lexer->line_offsets->items[line];
    unsigned int abs_col = i + col;

    line_no_len = printf(" %d", line + 1);
    printf(" |  ");

    while (lexer->src[i] != '\n' && lexer->src[i] != 0) {
#ifdef INF_SHOW_COLORS
        if (i == abs_col)
            // color
            printf("%s", color);
#endif
        putchar(lexer->src[i++]);
#ifdef INF_SHOW_COLORS
        if (i - abs_col == mark_length)
            // reset color
            printf(RESET);
#endif
    }
    // print message
#ifdef INF_SHOW_COLORS
    printf("\n%*s |  %*s%s^%s\n", line_no_len, "", col, "", color, RESET);
#else
    printf("\n%*s |  %*s^\n", line_no_len, "", col, "");
#endif
}

void __log_msg(Caller caller, LogLevel level, const char *msg, va_list argv) {
    char *format;
    alsprintf(&format, "%s\n", msg); // add \n to the message

    // log message
#ifdef INF_DEBUG
    printf("[%s] ", caller_type_to_str(caller));
#endif
#ifdef INF_SHOW_COLORS
    printf("%s", get_log_level_color(level));
#endif
    printf("%s: ", log_level_to_str(level));
    vprintf(format, argv);
#ifdef INF_SHOW_COLORS
    printf(RESET);
#endif

    free(format);
}

void __log_with_trace(Caller caller, const Lexer *lexer, LogLevel level, unsigned int line, unsigned int col,
                      int mark_length, const char *msg, va_list argv) {
    __log_msg(caller, level, msg, argv);
    new_log_curr_line(lexer, get_log_level_color(level), line, col, mark_length);
    puts("");
}

void log_success(Caller caller, const char *format, ...) {
#ifdef INF_DEBUG
    va_list args;
    va_start(args, format);
    __log_msg(caller, SUCCESS, format, args);
    va_end(args);
#endif
}

void log_debug(Caller caller, const char *format, ...) {
#ifdef INF_DEBUG
    va_list args;
    va_start(args, format);
    __log_msg(caller, DEBUG, format, args);
    va_end(args);
#endif
}

void log_info(Caller caller, const char *format, ...) {
    va_list args;
    va_start(args, format);
    __log_msg(caller, INFO, format, args);
    va_end(args);
}

void log_warning(Caller caller, const char *format, ...) {
    va_list args;
    va_start(args, format);
    __log_msg(caller, WARNING, format, args);
    va_end(args);
}

// logs an error without exiting
void log_error(Caller caller, const char *format, ...) {
    va_list args;
    va_start(args, format);
    __log_msg(caller, ERROR, format, args);
    va_end(args);
}

// same as log_error, but exist the program
void log_exception(Caller caller, const char *format, ...) {
    va_list args;
    va_start(args, format);
    __log_msg(caller, ERROR, format, args);
    va_end(args);
    exit(1);
}

// logs error with trace *without exiting*
void log_error_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                          const char *format, ...) {
    va_list args;
    va_start(args, format);
    __log_with_trace(caller, lexer, ERROR, line, col, mark_length, format, args);
    va_end(args);
}

// logs error with trace and exist with code 1
void log_exception_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                              const char *format, ...) {
    va_list args;
    va_start(args, format);
    __log_with_trace(caller, lexer, ERROR, line, col, mark_length, format, args);
    va_end(args);
    exit(1);
}

void log_warning_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                            const char *format, ...) {
    va_list args;
    va_start(args, format);
    __log_with_trace(caller, lexer, WARNING, line, col, mark_length, format, args);
    va_end(args);
}

void throw_memory_allocation_error(Caller caller) {
    log_error(caller, "Can't allocate memory.");
}

/** Unicode */
void log_raw_debug(Caller caller, const char *color, const char *format, ...) {
    va_list args;
    va_start(args, format);
#ifdef INF_DEBUG
    printf("[%s] ", caller_type_to_str(caller));
#endif
#ifdef INF_SHOW_COLORS
    if (color)
        printf("%s", color);
#endif
    vprintf(format, args);
#ifdef INF_SHOW_COLORS
    printf(RESET);
#endif
    va_end(args);
}

void set_unicode_mode() {
    _setmode(_fileno(stdout), _O_U16TEXT); // set unicode mode
}

void set_ascii_mode() {
    _setmode(_fileno(stdout), _O_TEXT); // set ascii mode
}

void print_unicode(const wchar_t *color, const wchar_t *format) {
    set_unicode_mode();
#ifdef INF_SHOW_COLORS
    wprintf(color);
#endif
    wprintf(format);
#ifdef INF_SHOW_COLORS
    wprintf(UNI_RESET);
#endif
    set_ascii_mode();
}
