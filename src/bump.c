#include "mm_internal.h"
#include <stdint.h>  /* SIZE_MAX */

void *bump_alloc(arena_t *arena, size_t size) 
{   
    /* Our chosen behavior: a zero-byte request returns NULL. */
    if(arena == NULL || size == 0) {
        return NULL;
    }

    /* Rounding adds MM_ALIGNMENT - 1 to size. Check first so that addition cannot overflow. */
    if(size > SIZE_MAX - (MM_ALIGNMENT - 1)) {
        return NULL;
    }

    size_t aligned_size = (size + MM_ALIGNMENT - 1) / MM_ALIGNMENT * MM_ALIGNMENT;

    /* now calculate how many bytes remain in the arena. */
    size_t available = (size_t)(arena->end - arena->next);

    if(aligned_size > available) {
        return NULL;
    }

    void *result = arena->next;

    /* reserve space for the allocation required of size + metadata. */
    arena->next += aligned_size;

    return result;
}