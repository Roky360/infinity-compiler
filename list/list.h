#ifndef INFINITY_COMPILER_LIST_H
#define INFINITY_COMPILER_LIST_H

#include <stdio.h>

typedef struct {
    void **items;
    unsigned int size;
    unsigned int item_size;
} List;

List *init_list(unsigned int item_size);

void list_dispose(List *list);

int list_is_empty(List *list);

void *list_get_last(List *list);

void *list_get_item(List *list, int index);

void *list_get_address_by_index(List *list, int index);

void list_push(List *list, void *item);

void *list_pop(List *list);

// inserts `item` at index `idx`
void list_insert(List *list, int idx, void *item);

void list_print_integers(const List *list);

void list_print(const List *list, char *(*print_item)(const void *));

#endif //INFINITY_COMPILER_LIST_H
