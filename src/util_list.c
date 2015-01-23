#include "util_list.h"

#include <stdlib.h>

List * List_new(void) {
    List * list = malloc(sizeof(List));
    list->size = 0;
    list->capacity = 16;
    list->items = malloc(list->capacity * sizeof(void *));
    return list;
}
void List_add(List * list, void * item) {
    // ensure capacity
    if (list->size >= list->capacity) {
        void ** old_items = list->items;
        size_t old_capacity = list->capacity;
        list->capacity = old_capacity * 2;
        list->items = malloc(list->capacity * sizeof(void *));
        for (size_t i = 0; i < old_capacity; i++)
            list->items[i] = old_items[i];
        free(old_items);
    }
    list->items[list->size++] = item;
}
void List_delete(List * list) {
    free(list->items);
    free(list);
}
