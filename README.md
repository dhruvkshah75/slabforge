# slabforge

Slabforge is an educational memory allocator written in C11. The project builds toward `my_malloc`, `my_free`, `my_realloc`, and `my_calloc` with libc-compatible semantics, using an `mmap`-backed arena rather than calling libc's allocation functions inside the allocator. It is designed to be built and tested one phase at a time.

## Planned hybrid allocator

Slabforge is one allocator with three backends. Its public API selects a backend based on the request; each backend manages part of an `mmap`-backed arena. The exact size thresholds and arena layout remain to be decided.

```text
                       Application
                           |
             my_malloc(size) / my_calloc(...)
                           |
               validate and select backend
                           |
        +------------------+------------------+
        |                  |                  |
   Small objects      Medium blocks      Large blocks
    Slab pools      Segregated bins     Buddy allocator
        |                  |                  |
   Reuse a slot       Find and split     Split a power-of-two
   from a page        a free block       block as needed
        |                  |                  |
        +------------------+------------------+
                           |
               return aligned memory

                   my_free(pointer)
                           |
               identify owning backend
                           |
        +------------------+------------------+
        |                  |                  |
   Return slot        Coalesce free      Merge free buddies
   to its pool        neighboring blocks when possible
```

Slabs hold equal-sized slots for small objects. Segregated bins group medium free blocks by size to narrow the search for a suitable block. The buddy allocator handles large blocks in power-of-two sizes, splitting and merging them as needed. The arena backs these strategies; it is not a separate allocation backend.

`my_calloc` will check for size overflow and zero the allocated memory. `my_realloc` will preserve existing data and may move an allocation between backends when its size changes. A heap checker will validate allocator invariants as the backends are implemented.


## Tests

Run `make test` from the project root. See the [test guide](test/README.md) for suite selection, direct runner usage, and build requirements.

