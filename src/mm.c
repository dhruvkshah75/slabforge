#include "mm.h"
#include "mm_internal.h"

/* The arena itself lives in the mmap region; this points to it. */
static arena_t *arena = NULL;

void *my_malloc(size_t size) 
{
    if(size == 0) return NULL;

    /* create the arena only when it is needed. */
    if(arena == NULL) {
        arena = arena_init();
        if(arena == NULL) return NULL;
    }

    return bump_alloc(arena, size);
}

void my_free(void *ptr)
{
    /* Individual allocations cannot be reclaimed by a bump allocator. */
    (void)ptr;
}
