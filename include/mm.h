#ifndef MM_H
#define MM_H

#include <stddef.h>

/* Allocate size bytes, or return NULL if the request cannot be satisfied. */
void *my_malloc(size_t size);

/* Release a previously allocated block. Does nothing if ptr is NULL. */
void my_free(void *ptr);

#endif