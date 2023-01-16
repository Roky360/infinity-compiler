#ifndef INFINITY_COMPILER_IO_H
#define INFINITY_COMPILER_IO_H

#include <stdio.h>

unsigned long file_size(const char *file_name);

char *read_file(const char *filename);

char *get_file_extension(char *filename);

char *alsprintf(char **stream, char *format, ...);

#endif //INFINITY_COMPILER_IO_H
