# Slabforge: three-day checkpoint plan

## Summary

This document is the project's working guide for the schedule, file inventory, allocator design, implementation tips, test gates, and suggested commit messages. Update it as decisions or results change.

We'll build together, completing and testing one checkpoint at a time. Each checkpoint ends with a reviewable commit. The core free-list allocator is the priority; slabs and the buddy allocator are scheduled as stretch goals if the first two days take longer than expected.

## Architecture and files

- `Makefile` — build the library, tests, and benchmark; provide sanitizer-enabled test and clean targets.
- `include/mm.h` — public declarations for `my_malloc`, `my_free`, `my_realloc`, and `my_calloc`.
- `src/mm_internal.h` — private arena, block, backend, and heap-checker declarations and shared constants.
- `src/arena.c` — map and initialize the arena and manage its bounds.
- `src/bump.c` — phase-one bump allocation.
- `src/freelist.c` — block metadata, splitting, and coalescing.
- `src/heap_check.c` — validate arena and backend invariants.
- `src/bins.c` — size-class free lists and suitable-block search.
- `src/slab.c` — fixed-size pools for small allocations.
- `src/buddy.c` — power-of-two allocation and buddy merging.
- `src/mm.c` — public API and backend dispatch.
- `test/test_basic.c` — deterministic API, alignment, boundary, and backend tests.
- `test/test_stress.c` — seeded randomized operations, data-pattern checks, and heap checks.
- `bench/bench.c` and `bench/RESULTS.md` — comparable workloads and recorded results.
- `README.md` — concise project overview and build instructions, updated as functionality lands.

The initial arena is a 1 MiB anonymous mapping. Keep allocator metadata in that mapping, with only the arena's entry pointer held outside it. Start with one allocation region; when adding slabs and buddies, reserve distinct, page-aligned regions so `free` can identify the backend by address. Document region sizes and capacity limits before implementing dispatch.

All returned pointers must be 16-byte aligned. Guard size arithmetic against overflow and return `NULL` on failure. Define zero-size behavior consistently in the public API documentation. `free(NULL)` is a no-op; failed `realloc` preserves the original allocation; `calloc` checks multiplication overflow and zeroes its result. Phase one intentionally has no-op free and does not yet provide full final API behavior.

## Checkpoints and commits

### Before day 1 — baseline

1. Add this guide and commit the existing repository structure and documentation.
2. Suggested commit: `docs: add three-day allocator implementation plan`.

### Day 1 — arena and reusable blocks

1. **Arena and bump allocator:** Add the public API, build targets, `mmap` arena, aligned bump allocation, and basic tests. Check initialization failure, alignment, stored data, and exhaustion.  
   Commit: `feat: add mmap arena and bump allocator`.
2. **Free-list allocator:** Add headers and footers, first-fit search, splitting, immediate coalescing, `realloc`, `calloc`, and initial heap checks. Test all coalescing directions, reuse, overflow, and API edge cases.  
   Commit: `feat: add coalescing free-list allocator`.

### Day 2 — correctness and faster search

3. **Stress and invariants:** Add reproducible randomized tests that verify payload patterns and run `heap_check()` after each operation. Run at least 10,000 operations under ASan and UBSan.  
   Commit: `test: add allocator stress and invariant checks`.
4. **Segregated bins:** Replace the single free list with size-class bins, search the matching and larger classes, and use a good-fit choice within a bin. Verify that splitting and coalescing update bin membership correctly; measure block visits against the previous approach.  
   Commit: `feat: organize free blocks into size-class bins`.

### Day 3 — specialized backends and results

5. **Slabs:** Add 4 KiB pages with 16-, 32-, 64-, and 128-byte slots. Track page ownership and free slots in arena metadata; route eligible requests to slabs and recover the correct pool from an address. Test reuse, alignment, pool exhaustion, and mixed allocations.  
   Commit: `feat: add slab pools for small allocations`.
6. **Buddy allocator:** Add a separately reserved, power-of-two region for large allocations. Split blocks to the required order and merge free buddies using XOR on region-relative offsets. Extend heap checks to verify order, bounds, free-list membership, and complete merging.  
   Commit: `feat: add buddy allocator for large allocations`.
7. **Benchmark and write-up:** Compare Slabforge and glibc using small-object, mixed-size, and long-lived plus short-lived workloads. Report elapsed time, operation count, and peak memory with the measurement method and limitations. Update the README and record results.  
   Commit: `bench: compare allocator workloads and document results`.

If time runs short, finish checkpoint 3 first, then checkpoint 4. Treat checkpoints 5–7 as stretch work; do not skip a test gate to start a later phase.

## Implementation guidance and acceptance

- Align metadata as well as payloads.
- Calculate split thresholds from the minimum valid free block.
- Unlink free neighbors before coalescing.
- Derive buddy addresses from offsets rather than raw pointers.
- Use a fixed random seed so stress failures can be reproduced.
- Keep each source file near 300 lines and add comments for invariants.

Each checkpoint passes when its targeted tests and all earlier tests pass with C11, `-Wall -Wextra -Werror -g`, ASan, and UBSan. From the free-list phase onward, stress tests call `heap_check()` after every operation. The project is complete when the full API, all planned backends, sanitizer tests, invariant checks, and benchmark results are present.

## Assumptions

- Development targets a POSIX-like system with anonymous `mmap` and `munmap`.
- This is a single-threaded educational allocator; thread safety and production use are outside the initial scope.
- Commits are made after each passing checkpoint, with their contents reviewed together.
