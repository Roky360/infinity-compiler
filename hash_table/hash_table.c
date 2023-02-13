#include "hash_table.h"
#include "../logging/logging.h"
#include <string.h>

HashTable *init_hash_table(unsigned capacity, void (*dispose_item)(void *item)) {
    HashTable *table = malloc(sizeof(HashTable));
    if (!table)
        throw_memory_allocation_error(COMPILER);

    table->entries = calloc(capacity, sizeof(TableEntry *));
    if (!table->entries)
        throw_memory_allocation_error(COMPILER);
    table->capacity = capacity;
    table->dispose_item = dispose_item;

    return table;
}

/**
 * Uses the Fowler–Noll–Vo hash function.
*/
unsigned int hash_func(HashTable *table, char *id) {
    size_t hash = FNV_offset_basis;
    while (*id) {
        hash ^= *id++;
        hash *= FNV_prime;
    }
    return hash % table->capacity;
}

// returns the TableEntry value (void *) that corresponds to the given value. returns NULL if not found.
void *hash_table_lookup(HashTable *table, char *id) {
    TableEntry *e = table->entries[hash_func(table, id)];
    while (e) {
        if (strcmp(e->id, id) == 0)
            return e->val;
        e = e->next;
    }
    return NULL;
}

// returns if insertion was successful
// if an entry with the same value exists, will return false (0)
int hash_table_insert(HashTable *table, char *id, void *value) {
    TableEntry *new_entry, *e;
    unsigned int idx;
    if (hash_table_lookup(table, id) != NULL) {
        return 0;
    }

    idx = hash_func(table, id);
    new_entry = init_table_entry(id, value);
    e = table->entries[idx];
    if (e == NULL) {
        table->entries[idx] = new_entry;
    } else {
        while (e->next != NULL)
            e = e->next;
        e->next = new_entry;
    }

    return 1;
}

int hash_table_remove(HashTable *table, char *id) {
    TableEntry *e, *prev;
    if (hash_table_lookup(table, id) == NULL) {
        return 0;
    }

    e = table->entries[hash_func(table, id)];
    if (e->next) {
        prev = e;
        while (strcmp(e->id, id) != 0) {
            prev = e;
            e = e->next;
        }
        prev->next = e->next;
    }
    table_entry_dispose(e, table->dispose_item);

    return 1;
}

void hash_table_dispose(HashTable *table) {
    int i;
    TableEntry *e;
    for (i = 0; i < table->capacity; i++) {
        e = table->entries[i];
        if (e) {
            table_entry_dispose(e, table->dispose_item);
        }
    }
    free(table);
}
