#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <stdarg.h>

// These functions return the number of bytes that we *wanted* to write,
// excluding the \0. If this number is >= than dest_size, the buffer was
// too small.
size_t nh_strlcpy(char *dest, const char *source, size_t dest_size);
size_t nh_vslprintf(char *dest, size_t dest_size, char *format, va_list ap);
size_t nh_slprintf(char *dest, size_t dest_size, char *format, ...) __attribute__ ((format (printf, 3, 4)));

#endif // UTIL_H
