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

void log_debug(Caller caller, const char *format, ...) {
    va_list args;
    char *msg_formatted;
    va_start(args, format);
    vprintf(alsprintf(&msg_formatted, "[%s] %s\n\n", caller_type_to_str(caller), format), args);
    free(msg_formatted);
    va_end(args);
}

void log_error(Caller caller, const char *msg) {
    log_debug(caller, msg);
    exit(1);
}

void log_warning(const Lexer *lexer, const char *msg) {
    log_curr_line(lexer);
    printf("[Warning] %s\n", msg);
}

void new_log_curr_line(const Lexer *lexer, unsigned int line, unsigned int col, int mark_length) {
    unsigned int line_no_len, i = 0;
    int start_line = *(int *) lexer->line_offsets->items[line];

    line_no_len = printf(" %d", line + 1);
    printf(" |  ");

    while (lexer->src[start_line + i] != '\n' && lexer->src[i] != 0) {
#ifdef INF_SHOW_COLORS
        if (i == col)
            // color
            printf(RED);
#endif
        putchar(lexer->src[start_line + i++]);
#ifdef INF_SHOW_COLORS
        if (i - col == mark_length)
            // reset color
            printf(RESET);
#endif
    }
    // print message
#ifdef INF_SHOW_COLORS
    printf("\n%*s |  %*s%s^%s\n", line_no_len, "", col, "", RED, RESET);
#else
    printf("\n%*s |  %*s^\n", line_no_len, "", col, "");
#endif
}

void __log_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                      char *msg, va_list argv) {
    char *format;
    alsprintf(&format, "%s\n", msg); // add \n to the message

    // log message
    printf("[%s] ", caller_type_to_str(caller));
    vprintf(format, argv);
    // log source code line
    new_log_curr_line(lexer, line, col, mark_length);

    free(format);
}

void message_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                        char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __log_with_trace(caller, lexer, line, col, mark_length, msg, args);
    puts("");
    va_end(args);
}

void new_exception_with_trace(Caller caller, const Lexer *lexer, unsigned int line, unsigned int col, int mark_length,
                              char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __log_with_trace(caller, lexer, line, col, mark_length, msg, args);
    va_end(args);
    exit(1);
}

void throw_memory_allocation_error(Caller caller) {
    log_error(caller, "Can't allocate memory.");
}

void log_raw_debug(Caller caller, const char *color, const char *format, ...) {
    va_list args;
    va_start(args, format);
#ifdef INF_SHOW_COLORS
    if (color)
        printf("%s", color);
#endif
    printf("[%s] ", caller_type_to_str(caller));
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
