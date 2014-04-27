#include <stddef.h>

// Two-level segregated fit (TLSF) is a data structure that supports
// dynamic storage allocation (such as malloc and free in C, or new
// and delete in C++) in constant time with low internal
// fragmentation.
//
// This file is the interface to a TLSF allocator that acts as an
// arena or memory pool, in that memory is not returned to the
// operating system until the TLSF allocator is finally destroyed.

struct tlsf_arena;

// tlsf_create uses an existing block of allocated memory to
// initialize a TLSF arena and some free blocks.
struct tlsf_arena * tlsf_create(void *, size_t);

void tlsf_augment(struct tlsf_arena *, void *, size_t);

// tlsf_malloc and tlsf_free are meant to mimic the interfaces of
// malloc(3) and free(3).
void * tlsf_malloc(struct tlsf_arena *, size_t);
void tlsf_free(struct tlsf_arena *, void *);
