#ifndef INFINITY_COMPILER_OPTIONS_PARSER_H
#define INFINITY_COMPILER_OPTIONS_PARSER_H

#define OPTIONS_FILE "../config/options.txt"

int parse_options(int argc, char **argv);

void op_help();

void op_debug(const char *mode);

void op_color(const char *mode);

#endif //INFINITY_COMPILER_OPTIONS_PARSER_H
