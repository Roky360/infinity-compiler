#ifndef INFINITY_COMPILER_LIST_H
#define INFINITY_COMPILER_LIST_H

typedef struct List {
    void **items;
    unsigned int size;
    unsigned int item_size;
} List;

/// Initializes an empty list.
/// \param item_size The size of each element in the list.
/// \return Pointer to the initialized list
List *init_list(unsigned int item_size);

/// Frees the memory of a list.
/// \param list
void list_dispose(List *list);

/// Checks if a list is empty
/// \param list
/// \return 1 if the list is empty, 0 otherwise.
int list_is_empty(List *list);

/// Get the last element of a list
/// \param list
/// \return The last element of the list
void *list_get_last(List *list);

void *list_get_item(List *list, int index);

/// Appends an item to the end of the list.
/// \param list The list to work with.
/// \param item The item to insert.
void list_push(List *list, void *item);

/// Removes and returns the last element of a list.
/// \param list
/// \return The last element.
void *list_pop(List *list);

/// Clears a list from items.
/// \param list The list to clear
/// \param free_content Whether to free the elements in the list
void list_clear(List *list, int free_content);

/// Inserts an item at an index.
/// \param list
/// \param idx Index to insert the new item.
/// \param item The item to insert.
void list_insert(List *list, int idx, void *item);

void list_print_integers(const List *list);

/// Print the contents of a list
/// \param list
/// \param item_to_string Pointer to function that returns a string representation an element in the list.
void list_print(const List *list, char *(*item_to_string)(const void *));

#endif //INFINITY_COMPILER_LIST_H
