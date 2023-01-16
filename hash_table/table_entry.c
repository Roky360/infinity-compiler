#include "table_entry.h"
#include "../logging/logging.h"
#include <stdlib.h>

TableEntry *init_table_entry(char *id, void *val) {
    TableEntry *e = malloc(sizeof(TableEntry));
    if (!e)
        throw_memory_allocation_error(COMPILER);
    e->id = id;
    e->val = val;
    e->next = NULL;
    return e;
}

void table_entry_dispose(TableEntry *entry, void (*dispose_item)(void *item)) {
    free(entry->id);
    if (dispose_item)
        dispose_item(entry->val);
    if (entry->next)
        table_entry_dispose(entry->next, dispose_item);
    free(entry);
}
