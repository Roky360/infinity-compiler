#ifndef INFINITY_COMPILER_STRING_SYMBOL_H
#define INFINITY_COMPILER_STRING_SYMBOL_H

#include <corecrt.h>

typedef struct StringSymbol {
    char *symbol_name; // formatted symbol name, as will appear in the data segment
    char *value; // value of the string
    size_t length;
} StringSymbol;

StringSymbol *init_string_symbol(char *id);

void string_symbol_dispose(void *s_symbol);

char *generate_string_symbol_id();

#endif //INFINITY_COMPILER_STRING_SYMBOL_H
