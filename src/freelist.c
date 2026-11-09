#include "mm_internal.h"

#include <stdint.h> /* SIZE_MAX */

/*
 * Find the first free block whose total size is at least required_size.
 * Return NULL if no block fits.
 */
block_header_t *find_free_block(arena_t *arena, size_t required_size)
{
    unsigned char *curr = arena->next;

    while(curr < arena->end) {
        block_header_t *block = (block_header_t *)curr;

        if(!block->allocated && block->size >= required_size) {
            return block;
        }

        /* Move onto to the beginning of the next block. */
        curr += block->size;
    }

    /* couldnt find a suitable block. */
    return NULL;
}

/*
 * Allocate size bytes from the arena using first fit algorithm.
 * Returns NULL if the request cannot be satisfied.
 */
void *freelist_alloc(arena_t *arena, size_t size)
{
    if(arena == NULL || size == 0) return NULL;

    /*
     * A block needs space for its header, payload and footer
     * Leave room for the addition used when rounding up as well.  
     */
    size_t overhead = BLOCK_HEADER_SIZE + BLOCK_FOOTER_SIZE;

    if(size > SIZE_MAX - overhead - (MM_ALIGNMENT - 1)) 
        return NULL;

    size_t required_size = ALIGN_UP(overhead + size);
    
    block_header_t *block = find_free_block(arena, required_size);
    if(block == NULL) return NULL;

    /* allocate the leftover block after the current sized block is given space */
    size_t original_size = block->size;
    size_t remaining_size = original_size - required_size;

    /*
     * A leftover block must have room for header, payload and footer
     * with at least 1 byte of payload and its total size must be aligned
     */
    size_t minimum_block_size = ALIGN_UP(overhead + 1);

    if(remaining_size >= minimum_block_size) {
        /* Create a new free block after the allocated portion. */
        unsigned char *remaining_start = (unsigned char *)block + required_size;

        block_header_t *remaining = (block_header_t *)remaining_start;
        remaining->size = remaining_size;
        remaining->allocated = 0;

        size_t *remaining_footer = (size_t *)(remaining_start + remaining_size - BLOCK_FOOTER_SIZE);
        *remaining_footer = remaining_size;

        /* splitting the block by adjusting its size. */
        block->size = required_size;
    }

    /*
     * If the remainder was too small, block->size still holds the
     * original size and the caller receives the entire block.
     */
    block->allocated = 1;

    size_t *footer = (size_t *)((unsigned char *)block + block->size - BLOCK_FOOTER_SIZE);
    *footer = block->size;

    /* The caller receives the payload, not the header. */
    return (unsigned char *)block + BLOCK_HEADER_SIZE;
}
