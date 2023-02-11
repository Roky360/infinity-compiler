#include "string_repository.h"
#include "../../logging/logging.h"
#include "../../config/globals.h"
#include "string_symbol.h"
#include <stdlib.h>

StringRepository *init_string_repository() {
    StringRepository *str_repo = malloc(sizeof(StringRepository));
    if (!str_repo)
        throw_memory_allocation_error(SEMANTIC_ANALYZER);

    str_repo->table = init_hash_table(STRING_TABLE_SIZE, string_symbol_dispose);
    str_repo->lst = init_list(sizeof(StringSymbol *));

    return str_repo;
}

void string_repository_dispose(StringRepository *str_repo) {
    hash_table_dispose(str_repo->table);
    free(str_repo->lst->items);
    free(str_repo->lst);
    free(str_repo);
}

void string_repository_add_string_identifier(StringRepository *str_repo, char *id) {
    StringSymbol *symbol = init_string_symbol(id);
    if (hash_table_insert(str_repo->table, id, symbol))
        list_push(str_repo->lst, symbol);
}

StringSymbol *string_repository_lookup(StringRepository *str_repo, char *id) {
    return (StringSymbol *) hash_table_lookup(str_repo->table, id);
}
