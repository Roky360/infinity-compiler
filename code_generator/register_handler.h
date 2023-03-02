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

/// Returns any available register. If no available register is found, return EAX by default
/// (and protect its value by pushing it).
/// \param reg_handler The register handler struct
/// \param fp Output file, in case `push` is needed
/// \return Name of the available register found.
char *register_handler_request_available_register(RegisterHandler *reg_handler, FILE *fp);

/// returns a specific requested register
/// \param reg_handler The register handler struct
/// \param fp Output file, in case `push` is needed
/// \param reg_name The name of the desired register
/// \return Name of the requested register.
char *register_handler_request_register(RegisterHandler *reg_handler, FILE *fp, char *reg_name);

/// Mark that a register is no longer in use.
/// This is called when a register is no longer in use in a certain process.
/// \param reg_handler The register handler struct
/// \param fp Output file, in case `push` is needed
/// \param reg_name The name of the register to free
void register_handler_free_register(RegisterHandler *reg_handler, FILE *fp, char *reg_name);

/// Returns if a register is an 8-bit register.
/// \param reg_name
/// \return 1 if the register's size is byte, and 0 otherwise.
int register_handler_is_register_byte(char *reg_name);

/// Returns the lower byte of a 32-bit register.
/// \param reg_name
/// \return
char *register_handler_get_lower_byte(char *reg_name);

#endif //INFINITY_COMPILER_REGISTER_HANDLER_H
