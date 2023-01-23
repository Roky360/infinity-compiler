#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config/globals.h"
#include "compiler/compiler.h"
#include "io/io.h"

/*
// TODO: add EOF proof to parser

 TODO: make `loop` support expressions, not just constants

// TODO: better logging system (add row and col to each token + add colors)
// TODO: in the new logging system, change that the logging method will accept va_list, instead if calling alsprintf before..

// TODO: support one-line functions :D
*/
int main(int argc, char **argv) {
    char *output_path;

    // check that target file is specified
    if (argc < 2) {
        printf("Please provide target file path as a command line argument.\n"
               "Usage: %s target_file.%s\n", argv[0], INPUT_EXTENSION);
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
        output_path = alsprintf(&output_path, DEFAULT_OUTPUT_PATH "%s", get_file_name(argv[1]));
        change_file_extension(output_path, OUTPUT_EXTENSION);
    }

    compiler_compile_file(argv[1], output_path);
    printf("Done\n");

    return 0;
}

// Evaluator issues

//#include <stdio.h>
//#include "expression_evaluator/exp_eval_backup.c"
//int main(int argc, char **argv) {
//    char *expression = calloc(100, sizeof(char));
//    while (1) {
//        gets(expression);
//        printf("%f\n", evaluate_expression(expression));
//    }
//}
