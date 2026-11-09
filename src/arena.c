#include "mm_internal.h"
#include <sys/mman.h>

/* Create and initialize the arena. Return NULL on failure. */
arena_t *arena_init(void) 
{
    void *mapping = mmap(NULL, ARENA_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // mapping is a private anonymous pages of ARENA_SIZE with no file backing 

    if(mapping == MAP_FAILED) {
        return NULL;
    }

    arena_t *arena = mapping;

    /* reserve enough space for the arena metadata such that the first allocation starts aligned. */
    size_t metadata_size = (sizeof(arena_t) + MM_ALIGNMENT - 1) / MM_ALIGNMENT * MM_ALIGNMENT;

    arena->next = (unsigned char *)mapping + metadata_size;
    arena->end = (unsigned char *)mapping + ARENA_SIZE;

    return arena;
}