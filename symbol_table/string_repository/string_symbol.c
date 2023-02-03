#include "string_symbol.h"
#include "../../logging/logging.h"
#include "../../io/io.h"
#include "../../code_generator/instruction_generators.h"
#include <stdlib.h>
#include <string.h>

StringSymbol *init_string_symbol(char *id) {
    StringSymbol *s_symbol = malloc(sizeof(StringSymbol));
    if (!s_symbol)
        throw_memory_allocation_error(SEMANTIC_ANALYZER);

    s_symbol->value = id;
    s_symbol->symbol_name = generate_string_symbol_id();
    s_symbol->length = strlen(id);

    return s_symbol;
}

void string_symbol_dispose(void *s_symbol) {
    free(((StringSymbol *) s_symbol)->symbol_name);
    free(s_symbol);
}

char *generate_string_symbol_id() {
    static int string_symbol_id = 0;
    char *format;
    return alsprintf(&format, STRING_FORMAT, string_symbol_id++);
}
