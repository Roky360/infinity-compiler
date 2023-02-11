#ifndef INFINITY_COMPILER_STRING_REPOSITORY_H
#define INFINITY_COMPILER_STRING_REPOSITORY_H

#include "../../hash_table/hash_table.h"
#include "../../list/list.h"
#include "string_symbol.h"

typedef struct StringRepository {
    HashTable *table;
    List *lst;
}StringRepository;

StringRepository *init_string_repository();

void string_repository_dispose(StringRepository *str_repo);

void string_repository_add_string_identifier(StringRepository *str_repo, char *id);

StringSymbol *string_repository_lookup(StringRepository *str_repo, char *id);

#endif //INFINITY_COMPILER_STRING_REPOSITORY_H
