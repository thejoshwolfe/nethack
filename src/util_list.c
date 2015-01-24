#include "util_list.h"

#include <stdlib.h>
#include <string.h>

List * List_new(void) {
    List * list = malloc(sizeof(List));
    list->size = 0;
    list->capacity = 16;
    list->items = calloc(list->capacity, sizeof(void *));
    return list;
}
void List_add(List * this, void * item) {
    // ensure capacity
    if (this->size >= this->capacity) {
        this->capacity *= 2;
        this->items = realloc(this->items, this->capacity * sizeof(void *));
    }
    this->items[this->size++] = item;
}
void List_delete(List * this) {
    free(this->items);
    free(this);
}


ByteBuffer * ByteBuffer_new(void) {
    ByteBuffer * list = malloc(sizeof(ByteBuffer));
    list->size = 0;
    list->capacity = 16;
    list->buffer = calloc(list->capacity, 1);
    return list;
}
static void ByteList_ensure_capacity(ByteBuffer * this, size_t new_capacity) {
    size_t better_capacity = this->capacity;
    while (better_capacity < new_capacity)
        better_capacity *= 2;
    if (better_capacity != this->capacity) {
        this->capacity = better_capacity;
        this->buffer = realloc(this->buffer, better_capacity);
    }
}
void ByteBuffer_write(ByteBuffer * this, void * buffer, size_t size_in_bytes) {
    ByteList_ensure_capacity(this, this->size + size_in_bytes);
    memcpy(this->buffer + this->size, buffer, size_in_bytes);
    this->size += size_in_bytes;
}
void ByteBuffer_delete(ByteBuffer * list) {
    free(list->buffer);
    free(list);
}
