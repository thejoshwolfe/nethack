#ifndef UTIL_LIST_H
#define UTIL_LIST_H

#include <stddef.h>

typedef struct {
    size_t size;
    size_t capacity;
    void ** items;
} List;

List * List_new(void);
void List_add(List * list, void * item);
void List_delete(List * list);

#endif // UTIL_LIST_H
