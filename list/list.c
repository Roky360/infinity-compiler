#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List *init_list(unsigned int item_size) {
    List *list = malloc(sizeof(List));
    if (!list) {
        printf("Can't allocate memory for list.\n");
        exit(1);
    }
    list->item_size = item_size;
    list->size = 0;
    list->items = NULL;

    return list;
}

void list_dispose(List *list) {
    int i;
    for (i = 0; i < list->size; i++)
        free(list->items[i]);
    free(list->items);
    free(list);
}

int list_is_empty(List *list) {
    return list->size == 0;
}

void *list_get_last(List *list) {
    return list->items[list->size - 1];
}

void *list_get_item(List *list, int index) {
    if (index > list->size - 1 || index < 0) {
        return NULL;
        // printf("List index %d out of range: 0..%d\n", index, list->size - 1);
        // exit(1);
    }
    return list->items[index];
}

void list_push(List *list, void *item) {
    if ((list->size)++ == 0) {
        list->items = calloc(1, list->item_size);
    } else {
        list->items = realloc(list->items, list->size * list->item_size);
    }
    list->items[list->size - 1] = item;
}

void *list_pop(List *list) {
    if (list->size == 0) {
        return NULL;
    }
    void *val = list->items[list->size - 1];
    list->items = realloc(list->items, --(list->size) * list->item_size);
    return val;
}

void list_clear(List *list, int free_content) {
    if (free_content)
        while (!list_is_empty(list))
            free(list_pop(list));
    else
        while (!list_is_empty(list))
            list_pop(list);
}

void list_insert(List *list, int idx, void *item) {
    if (idx > list->size - 1 || idx < 0) {
        printf("List index %d out of range: 0..%u\n", idx, list->size - 1);
        exit(1);
//        return;
    }

    if ((list->size)++ == 0) {
        list->items = calloc(1, list->item_size);
    } else {
        list->items = realloc(list->items, list->size * list->item_size);
    }

    memmove(&list->items[idx + 1], &list->items[idx], (list->size - idx) * list->item_size);

    list->items[idx] = item;
}

void list_print_integers(const List *list) {
    printf("[");
    for (int i = 0; i < list->size; i++) {
        printf("%d%s", *(int *) (list->items[i]), i == list->size - 1 ? "" : ", ");
    }
    printf("]\n");
}

void list_print(const List *list, char *(*print_item)(const void *)) {
    printf("[");
    for (int i = 0; i < list->size; i++) {
        printf("%s%s", print_item(list->items[i]), i == list->size - 1 ? "]\n" : ", ");
    }
}
