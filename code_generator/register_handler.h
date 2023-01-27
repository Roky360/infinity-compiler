#ifndef INFINITY_COMPILER_REGISTER_HANDLER_H
#define INFINITY_COMPILER_REGISTER_HANDLER_H

#include "../hash_table/hash_table.h"
#include <stdio.h>

/** Registers */
#define REGISTER_COUNT 16

#define EAX "eax"
#define EBX "ebx"
#define ECX "ecx"
#define EDX "edx"
#define ESI "esi"
#define EDI "edi"
#define EBP "ebp"
#define ESP "esp"

#define AH "ah"
#define AL "al"
#define BH "bh"
#define BL "bl"
#define CH "ch"
#define CL "cl"
#define DH "dh"
#define DL "dl"

extern char *reg_names[REGISTER_COUNT];

typedef struct {
    char *name;
    int available; // whether the register is available to use
    int uses; // counts the number of uses
} Register;

typedef struct RegisterHandler {
    HashTable *registers_table; // all the registers as a hash table, for efficient search
    Register *registers_array[REGISTER_COUNT]; // all the registers as a list, for faster linear search
} RegisterHandler;

RegisterHandler *init_register_handler();

Register *init_register(char *name);

void register_handler_dispose(RegisterHandler *reg_handler);

void dispose_register(void *reg);

char *register_handler_request_available_register(RegisterHandler *reg_handler, FILE *fp);

char *register_handler_request_register(RegisterHandler *reg_handler, FILE *fp, char *reg_name);

void register_handler_free_register(RegisterHandler *reg_handler, FILE *fp, char *reg_name);

int register_handler_is_register_byte(char *reg_name);

char *register_handler_get_lower_byte(char *reg_name);

#endif //INFINITY_COMPILER_REGISTER_HANDLER_H
