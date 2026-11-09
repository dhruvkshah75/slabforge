# Slabforge

Slabforge is an educational memory allocator written in C11. The project builds toward `my_malloc`, `my_free`, `my_realloc`, and `my_calloc` with libc-compatible semantics, using an `mmap`-backed arena rather than calling libc's allocation functions inside the allocator. It is designed to be built and tested one phase at a time.

## How the allocator will work

1. **Arena and bump allocation:** Map an arena (initially 1 MiB) and advance a cursor for each allocation. Round sizes so returned pointers are 16-byte aligned. Freeing is initially a no-op.
2. **Free list:** Track blocks with headers and boundary-tag footers. Find a suitable free block using first fit, split blocks when useful, and immediately coalesce adjacent free blocks on free. Reallocation initially allocates, copies, and frees; calloc allocates and zeroes memory.
3. **Segregated bins:** Organize free blocks by size class. Search the matching bin and then larger bins, choosing a good fit to reduce unnecessary block visits.
4. **Slab pools:** Serve small objects from pages divided into equal-sized slots, with a free-slot list for each size class. Individual slots need no per-object header.
5. **Buddy allocator:** Serve large allocations from a region divided into power-of-two blocks. Split blocks in half to satisfy requests, and merge free buddies; calculate a buddy by XORing the block's offset with its size.

The final dispatcher will route requests to the appropriate backend. A heap checker will validate allocator invariants from phase 2 onward, including the consistency of free blocks and coalescing. The exact boundaries between backends will be chosen during implementation.

## Tests

Run `make test` from the project root. See the [test guide](test/README.md) for suite selection, direct runner usage, and build requirements.

