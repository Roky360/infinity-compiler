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
    // check that target file is specified
    if (argc < 2) {
        printf("Please provide target file path as a command line argument.\n");
        exit(0);
    }
    // check file extension
    if (strcmp(get_file_extension(argv[1]), EXTENSION) != 0) {
        printf("File extension not supported. Must be *.%s files only.\n", EXTENSION);
        exit(0);
    }

    compiler_compile_file(argv[1]);
    printf("Done\n");

    return 0;
}

// Evaluator issues
// TODO: postfix - FIX

//#include <stdio.h>
//#include "expression_evaluator/exp_eval_backup.c"
//int main(int argc, char **argv) {
//    char *expression = calloc(100, sizeof(char));
//    while (1) {
//        gets(expression);
//        printf("%f\n", evaluate_expression(expression));
//    }
//}
