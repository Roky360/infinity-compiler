#ifndef INFINITY_COMPILER_TABLE_ENTRY_H
#define INFINITY_COMPILER_TABLE_ENTRY_H

/// An entry of a hash table
typedef struct TableEntry {
    char *id;
    void *val;
    struct TableEntry *next;
} TableEntry;

TableEntry *init_table_entry(char *id, void *val);

void table_entry_dispose(TableEntry *entry, void (*dispose_item)(void *item));

#endif //INFINITY_COMPILER_TABLE_ENTRY_H
