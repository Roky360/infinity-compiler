#ifndef INFINITY_COMPILER_HASH_TABLE_H
#define INFINITY_COMPILER_HASH_TABLE_H

#include "table_entry.h"

#define FNV_offset_basis 14695981039346656037
#define FNV_prime 1099511628211

typedef struct HashTable {
    TableEntry **entries;
    unsigned int capacity;
    void (*dispose_item)(void *item);
} HashTable;

HashTable *init_hash_table(unsigned capacity, void (*dispose_item)(void *item));

unsigned int hash_func(HashTable *table, char *id);

void *hash_table_lookup(HashTable *table, char *id);

int hash_table_insert(HashTable *table, char *id, void *value);

int hash_table_remove(HashTable *table, char *id);

void hash_table_dispose(HashTable *table);

#endif //INFINITY_COMPILER_HASH_TABLE_H
