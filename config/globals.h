#ifndef INFINITY_COMPILER_GLOBALS_H
#define INFINITY_COMPILER_GLOBALS_H

#include "../list/list.h"
#include "../token/token.h"
#include "../hash_table/hash_table.h"

//#define _GNU_SOURCE
#define INF_DEBUG
#define EXTENSION "txt"
#define DEFAULT_ROOT_FUNCTION_NAME "main"

/** Defines */
#define SYMBOL_TABLE_SIZE 199
#define SCOPE_SIZE 23

/** Macros */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define ARRLEN(a) (sizeof(a) / sizeof((a)[0]))

/** Global Variables */
extern TokenType data_types[];
extern int data_types_len;

void init_globals();

void clean_globals();

#endif //INFINITY_COMPILER_GLOBALS_H
