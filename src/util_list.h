#ifndef UTIL_LIST_H
#define UTIL_LIST_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t size;
    size_t capacity;
    void ** items;
} List;

List * List_new(void);
void List_add(List * this, void * item);
void List_delete(List * this);

typedef struct {
    size_t size;
    size_t capacity;
    uint8_t * buffer;
} ByteBuffer;

ByteBuffer * ByteBuffer_new(void);
void ByteBuffer_write(ByteBuffer * this, void * buffer, size_t size_in_bytes);
void ByteBuffer_delete(ByteBuffer * this);

#endif // UTIL_LIST_H
