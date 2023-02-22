#ifndef INFINITY_COMPILER_IO_H
#define INFINITY_COMPILER_IO_H

#include <stdio.h>

unsigned long file_size(const char *file_name);

char *read_file(const char *filename);

char *get_file_extension(const char *filename);

const char *get_file_name(const char *path);

char *alsprintf(char **stream, const char *format, ...);

char *change_file_extension(char *filename, const char *new_ext);

void write_to_file(FILE *fp, const char *format, ...);

#endif //INFINITY_COMPILER_IO_H
