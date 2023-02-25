#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config/globals.h"
#include "compiler/compiler.h"
#include "io/io.h"

/*
 * TODO: handle duplicate variables on different scopes (x0, x1, ...)
 TODO: separate table initializers to files, by module
*/
int main(int argc, char *argv[]) {
    char *output_path;
    // check that target file is specified
    if (argc < 2) {
        printf("Please provide target file path as a command line argument.\n"
               "Usage: %s target_file_path.%s [output_file_path.%s]\n",
               get_file_name(argv[0]), INPUT_EXTENSION, OUTPUT_EXTENSION);
        exit(0);
    }
    // check file extension
    if (strcmp(get_file_extension(argv[1]), INPUT_EXTENSION) != 0) {
        printf("File extension not supported. Must be *.%s files only.\n", INPUT_EXTENSION);
        exit(0);
    }
    // check for output path
    if (argc >= 3) {
        output_path = argv[2];
        if (strcmp(get_file_extension(output_path), OUTPUT_EXTENSION) != 0) {
            printf("Output file must be *.%s only.\n", OUTPUT_EXTENSION);
            exit(0);
        }
    } else {
        output_path = change_file_extension(strdup(argv[1]), OUTPUT_EXTENSION);
    }

    compiler_compile_file(argv[1], output_path);

    return 0;
}
