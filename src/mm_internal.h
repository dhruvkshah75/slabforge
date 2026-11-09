// private header file where the allocator source files can share the definition of arena and functions that operate on it 

#ifndef MM_INTERNAL_H
#define MM_INTERNAL_H

#include <stddef.h>
#include <stdbool.h>

#define ARENA_SIZE (1024 * 1024)
#define MM_ALIGNMENT 16

typedef struct {
    unsigned char *next;   /* next points to the first byte available for the next allocation. */
    unsigned char *end;    /* end points one byte past the end of the mapped region. */
} arena_t;

typedef struct {
    size_t size;           /* size = total block size */
    int allocated;         /* allocated = 1 when in use OR = 0 when not in use. */
} block_header_t;

/* Round a size up to a multiple of MM_ALIGNMENT. */
#define ALIGN_UP(size) \
    (((size) + MM_ALIGNMENT - 1) / MM_ALIGNMENT * MM_ALIGNMENT)

/* Reserve an aligned amount of space for the header. */
#define BLOCK_HEADER_SIZE ALIGN_UP(sizeof(block_header_t))

/* Every block has a size_t boundary tag at its end. */
#define BLOCK_FOOTER_SIZE sizeof(size_t)

arena_t *arena_init(void);

void *bump_alloc(arena_t *arena, size_t size);

block_header_t *find_free_block(arena_t *arena, size_t required_size);

void *freelist_alloc(arena_t *arena, size_t size);

#endif



/*
    ┌────────────────┬─────────────────────────────────────────────┐
    │ arena metadata │              one large free block           │
    └────────────────┴─────────────────────────────────────────────┘
                     ^                                             ^
                     block start                                  block end

    Header                     Payload                     Footer
    ┌─────────────────────┬─────────────────────────┬─────────────────────┐
    │ size: 64            │ User's data goes here   │ size: 64            │
    │ allocated: 1        │                         │                     │
    └─────────────────────┴─────────────────────────┴─────────────────────┘

    both the header and footer store the same data that is the block_header_t 
    this helps the allocator needs to know the previous blocks size to merge it 
*/