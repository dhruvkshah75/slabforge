// private header file where the allocator source files can share the definition of arena and functions that operate on it 

#ifndef MM_INTERNAL_H
#define MM_INTERNAL_H

#include <stddef.h>

#define ARENA_SIZE (1024 * 1024)
#define MM_ALIGNMENT 16

typedef struct {
    unsigned char *next;   /* next points to the first byte available for the next allocation. */
    unsigned char *end;    /* end points one byte past the end of the mapped region. */
} arena_t;

/* Create and initialize the arena. Return NULL on failure. */
arena_t *arena_init(void);

/* Allocate size bytes from the arena. Return NULL on failure. */
void *bump_alloc(arena_t *arena, size_t size);


#endif