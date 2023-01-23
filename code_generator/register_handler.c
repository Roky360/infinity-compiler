#include "register_handler.h"
#include "../logging/logging.h"
#include "../io/io.h"
#include "instruction_generators.h"
#include <stdlib.h>
#include <string.h>

char *reg_names[REGISTER_COUNT] = {EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP};

RegisterHandler *init_register_handler() {
    int i;
    Register *reg;
    RegisterHandler *reg_handler = malloc(sizeof(RegisterHandler));
    if (!reg_handler)
        throw_memory_allocation_error(CODE_GENERATOR);

    reg_handler->registers_table = init_hash_table(REGISTER_COUNT * 2, dispose_register);
    for (i = 0; i < REGISTER_COUNT; i++) {
        reg = init_register(reg_names[i]);
        hash_table_insert(reg_handler->registers_table, strdup(reg_names[i]), reg);
        reg_handler->registers_array[i] = reg;
    }

    return reg_handler;
}

Register *init_register(char *name) {
    Register *reg = malloc(sizeof(Register));
    if (!reg)
        throw_memory_allocation_error(CODE_GENERATOR);
    reg->name = name;
    reg->available = 1;
    reg->uses = 0;
    return reg;
}

void register_handler_dispose(RegisterHandler *reg_handler) {
    hash_table_dispose(reg_handler->registers_table);
    free(reg_handler);
}

void dispose_register(void *reg) {
    free((Register *) reg);
}

// returns any available register
char *register_handler_request_available_register(RegisterHandler *reg_handler, FILE *fp) {
    int i;
    Register *reg;
    for (i = 0; i < REGISTER_COUNT; i++) {
        reg = reg_handler->registers_array[i];
        if (reg->available) {
            return reg->name;
        }
    }
    // all registers already in use, return EAX by default
    write_to_file(fp, PUSH, reg_handler->registers_array[0]->name);
    reg_handler->registers_array[0]->uses += 1;
    return reg->name;
}

// returns a specific requested register
char *register_handler_request_register(RegisterHandler *reg_handler, FILE *fp, char *reg_name) {
    Register *reg = (Register *) hash_table_lookup(reg_handler->registers_table, reg_name);
    if (!reg)
        return NULL;

    if (!reg->available) {
        // register already in use
        write_to_file(fp, PUSH, reg_name);
    }
    reg->uses += 1;
    reg->available = 0;

    return reg->name;
}

// mark that a register is no longer in use
void register_handler_free_register(RegisterHandler *reg_handler, FILE *fp, char *reg_name) {
    Register *reg = (Register *) hash_table_lookup(reg_handler->registers_table, reg_name);
    reg->uses -= 1;
    if (reg->uses > 0) {
        write_to_file(fp, POP, reg_name);
    } else {
        reg->available = 1;
    }
}
