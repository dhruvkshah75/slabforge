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

    /* The first free block uses the remaining arena space. Round down
     * so its total size is a multiple of 16.
     */
    size_t available = (size_t)(arena->end - arena->next);
    size_t block_size = (available) / MM_ALIGNMENT * MM_ALIGNMENT;

    /* This makes arena->end mark the end of the usable block region. */
    arena->end = arena->next + block_size;

    /* place the block header at the start of the usable region. */
    block_header_t *block = (block_header_t *)arena->next;
    block->size = block_size;
    block->allocated = 0;
    
    /* create the footer which stores the block size of this block. */
    size_t *footer = (size_t *)(arena->next + block_size - BLOCK_FOOTER_SIZE);
    *footer = block_size;

    return arena;
}