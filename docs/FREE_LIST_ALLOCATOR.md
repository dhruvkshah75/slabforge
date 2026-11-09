# Understanding the free-list allocator

This document describes the **planned** second checkpoint of Slabforge. The current bump allocator hands out memory by moving a cursor forward; its `my_free` does nothing. The free-list allocator will keep track of blocks so that memory can be freed and reused. The exact metadata layout is still a design choice to settle during implementation.

## The arena and its blocks

The arena is the region obtained from `mmap`. After the arena's own metadata and any alignment padding, the allocator divides the usable space into consecutive blocks:

```text
low addresses                                                  high addresses
┌──────────────────────────────────────────────────────────────┐
│ arena metadata │ block A: used │ block B: free │ block C: used │
└──────────────────────────────────────────────────────────────┘
```

At initialization, the usable region begins as one large free block. As allocations arrive, the allocator can split that block into smaller blocks. When blocks are freed, neighboring free blocks can be joined again.

A block has three conceptual parts:

```text
              total block size: 64 bytes
┌───────────────┬──────────────────────────┬───────────────┐
│    header     │         payload          │    footer     │
└───────────────┴──────────────────────────┴───────────────┘
^               ^                                           ^
block           pointer given to caller                     next block begins
```

- The **header** records the total block size and whether the block is allocated. The total includes the header, payload, padding, and footer.
- The **payload** is the space a caller may use. The allocator may reserve more space than requested to satisfy alignment or minimum-block requirements. A caller may use only the number of bytes it requested.
- The **footer**, or boundary tag, records enough information to find the start and status of the preceding block. A simple design repeats the size and allocation state from the header. It is allocator metadata, not space available to the caller.

One possible starting point for the header is:

```c
typedef struct {
    size_t size;    /* Total bytes in this block, including metadata. */
    int allocated;  /* 1 if in use; 0 if free. */
} block_header_t;
```

The compiler may add padding to this structure. Do not assume `sizeof(block_header_t)` is exactly 16 on every system. The footer representation and the amount of header space reserved must be chosen consistently before implementing block operations.

## Alignment and minimum block size

Every pointer returned to a caller must be aligned to 16 bytes. To make that work, choose a block layout where the first block starts aligned, the payload offset from its start is a multiple of 16, and every total block size is a multiple of 16. Then the next block and its payload will also start at the required alignment. Round sizes up carefully and check for overflow **before** adding alignment padding or metadata.

For example, suppose an implementation reserves 16 bytes for a header and 8 bytes for a footer. A request for 24 bytes needs at least 48 bytes in total: 16 for the header, 24 for the requested payload, and 8 for the footer. Since 48 is already a multiple of 16, no additional block padding is needed. These metadata sizes are an illustration, not a promise about C structure sizes on every platform.

A split is useful only if the remaining block can hold its own header, footer, any free-list links, and a minimally useful aligned payload. Compute that minimum from the chosen layout rather than relying on an unexplained number. If the remainder is too small, give the caller the entire block instead.

## Finding space: first fit

For a request, calculate the aligned total block size including metadata. Then search from the beginning for the **first free block** that is large enough:

```text
request: 32 bytes of payload
blocks:  [used] [free, too small] [used] [free, fits] [free, fits]
                                             ^
                                     first suitable block
```

If no suitable block exists, return `NULL` without changing existing allocations. Check for overflow while calculating the required size, before searching.

There are two ways to organize the search:

- An **implicit list** walks every physical block, using its size to find the next one and skipping allocated blocks.
- An **explicit free list** links only free blocks together, typically storing link pointers in their unused payload. It needs a list head in the arena metadata and careful updates when blocks are allocated, split, freed, or merged.

Both support first fit. The checkpoint describes a free list; settle on the representation before writing it. Later size-class bins will organize free blocks into multiple lists. Physical neighbors in memory and neighbors in an explicit free list are different concepts.

## Splitting a block

If a suitable free block is larger than needed, divide it into an allocated block and a smaller free block:

```text
before: [                    one free block                    ]
after:  [ allocated block ][           remaining free block    ]
```

Write consistent headers and footers for both resulting blocks. If using an explicit free list, remove or update the original entry and ensure the remainder appears in the list exactly once. If the remainder cannot form a valid block, allocate the whole original block.

## Freeing and coalescing

`my_free(ptr)` receives the payload pointer returned by the allocator. For a valid pointer, the allocator uses the agreed payload offset to locate its header, marks the block free, and checks its physical neighbors. `my_free(NULL)` does nothing.

The next block starts at the current block's start plus its total size. The previous block's footer is immediately before the current block; its recorded size identifies the previous block's start. Check arena boundaries **before** accessing either neighbor.

There are four cases:

```text
previous     current       next       action
used/edge    newly free    used/edge  keep the current block
free         newly free    used/edge  merge with previous
used/edge    newly free    free       merge with next
free         newly free    free       merge all three
```

For example:

```text
before free: [ free A ][ used B ][ free C ]
after free:  [         one larger free block         ]
```

This joining is called **coalescing**. Update the merged block's header and footer with its new total size and free state. With an explicit list, unlink free neighbors before merging, then insert the resulting block once. Immediate coalescing means two free physical neighbors should never remain separate after a completed free operation.

## Realloc and calloc

- `my_realloc(ptr, new_size)` can initially allocate a new block, copy the smaller of the old payload capacity and the new requested size, then free the old block. If the new allocation fails, leave the old block and its data intact. Handle `ptr == NULL` and zero size according to the public API's documented policy.
- `my_calloc(count, size)` first checks whether `count * size` would overflow. If it does, return `NULL`; otherwise allocate the resulting number of bytes and zero them before returning.

## Checking the heap

A `heap_check()` function will walk the arena and verify invariants after operations in debug tests. It should check that blocks stay inside the usable region, total sizes are valid and aligned, payload addresses are aligned, headers and footers agree, and adjacent blocks are not both free. If there is an explicit free list, also check that its entries are valid free blocks and that every free block appears exactly once.

Useful tests include exact fits, splits with valid and too-small remainders, reusing a freed block, all four coalescing cases, exhaustion, size overflow, alignment, and preserving data across `realloc`. Randomized tests should use a repeatable seed and check payload patterns and heap invariants after every operation.
