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

/// Initializes an empty hash table.
/// \param capacity Max capacity of the list
/// \param dispose_item Pointer to function that frees an item in the table
/// \return Pointer to the hash table
HashTable *init_hash_table(unsigned capacity, void (*dispose_item)(void *item));

/// A variation of the Fowler–Noll–Vo hash function.
/// \param table
/// \param id Id to search in the table.
/// \return Index in the table of the matching item
unsigned int hash_func(HashTable *table, char *id);

/// Searches an item in the table, by its id.
/// \param table
/// \param id Id of the desired item
/// \return The item if found, or NULL if not found
void *hash_table_lookup(HashTable *table, char *id);

/// Insert an item to the table
/// \param table
/// \param id Item identifier
/// \param value Item value
/// \return If the insertion was successful: 0 if an item with the same id already exists (insertion failed),
/// 1 otherwise (success)
int hash_table_insert(HashTable *table, char *id, void *value);

/// Removes an item from the table.
/// \param table
/// \param id Identifier of the item to remove.
/// \return If the removal was successful (will return false if the item does not exist)
int hash_table_remove(HashTable *table, char *id);

/// Free the memory taken by the table.
/// \param table
void hash_table_dispose(HashTable *table);

#endif //INFINITY_COMPILER_HASH_TABLE_H
