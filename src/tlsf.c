#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "tlsf-internals.h"

int is_valid_block_size(const size_t n) {
  // blocks have positive size:
  return ((n > 0) &&
  // blocks are 2*n*word_bytes in size for some positive n
          (0 == n % (2 * word_bytes)));
}

// Returns 1 if this block has no right neighbor, 0 otherwise.
int block_get_end(const struct block * const x) {
  return x->size & ((size_t)1);
}

void block_set_end(struct block * const x, int b) {
  if (b) {
    x->size |= (size_t)1;
  } else {
    x->size &= ~((size_t)1);
  }
}

// Returns the size of this block in bytes.
size_t block_get_size(const struct block * const x) {
  const size_t ans = x->size & ~((size_t)1);
  assert (is_valid_block_size(ans));
  return ans;
}

void block_set_size(struct block * const x, const size_t n) {
  assert (is_valid_block_size(n));
  const int old_end = block_get_end(x);
  x->size = n;
  block_set_end(x, old_end);
}

// Returns 1 if this block is free, 0 otherwise
int block_get_freedom(const struct block * const x) {
  return ((size_t)(x->left) & ((size_t)1));
}

void block_set_freedom(struct block * const y, const int f) {
  struct block ** z = &y->left;
  if (f) {
    *z = (struct block *)((size_t)(*z) | ((size_t)1));
  } else {
    *z = (struct block *)((size_t)(*z) & (~((size_t)1)));
  }
  assert (f == block_get_freedom(y));
}

struct block * block_get_left(struct block * const x) {
  return (struct block *)(((size_t)x->left) & (~((size_t)1)));
}

struct block * block_get_right(struct block * const x) {
  if (block_get_end(x)) return NULL;
  return (struct block *)((char *)(x->payload) + block_get_size(x));
}
  
void block_set_left(struct block * const x, struct block * const y) {
  const int freedom = block_get_freedom(x);
  x->left = y;
  block_set_freedom(x, freedom);
}

void coalesce_detached_blocks(struct block * const x, struct block * const y) {
  assert (y == block_get_right(x));

  struct block * const z = block_get_right(y);
  if (NULL != z) block_set_left(z, x);

  block_set_size(x, block_get_size(x) + sizeof(struct block) + block_get_size(y));
  block_set_end(x, block_get_end(y));
  assert (block_get_right(x) == z);
  assert (block_get_size(x) > 0);
}

// takes a number of bytes and returns a number of bytes
size_t round_up_to_block_size(const size_t n) {
  size_t ans = 2*word_bytes * (n/(2*word_bytes) + ((n & (2*word_bytes - 1)) > 0));
  if (ans < 2*word_bytes) ans = 2*word_bytes;
  assert (is_valid_block_size(ans));
  return ans;
}

size_t round_down_to_block_size(const size_t n) {
  // shift is the logarithm of 2 * word_bytes, which is the quantum
  // of block sizes.
  static const size_t shift = word_log - 2;
  const size_t ans = (n >> shift) << shift;
  assert (is_valid_block_size(ans));
  return ans;
}

// Downsize a block of size b to size n (or a few bytes larger). b
// must be free but not in any free list. After downsizing, any
// remainder is turned into a new block and returned.
struct block * block_split_detached(struct block * const b, const size_t n) {
  assert (1 == block_get_freedom(b));
  assert (NULL == b->payload[0]);
  assert (NULL == b->payload[1]);

  // Round up n to a multiple of 2*word_bytes:
  const size_t size = round_up_to_block_size(n);

  // Check that we can downsize:
  assert (block_get_size(b) >= size);
  if (block_get_size(b) <= size + sizeof(struct block) + 2*word_bytes) {
    // After downsizing, the remainder will be too small to be a block
    // on its own, so we keep it with b.
    return NULL;
  }

  // Set up the remainder block
  struct block * const ans = (struct block *)((char *)(b->payload) + size);
  block_set_left(ans, b);
  block_set_freedom(ans, 1);
  block_set_size(ans, block_get_size(b) - size - sizeof(struct block));
  block_set_end(ans, block_get_end(b));
  // Link the remainder block in with the coalescing list.
  struct block * const c = block_get_right(b);
  if (NULL != c) block_set_left(c, ans);
  // Set the remainder block in no free list yet
  ans->payload[0] = NULL;
  ans->payload[1] = NULL;

  // Reset b to its new size and blace in the coalescing list
  block_set_size(b, size);
  block_set_end(b, 0);

  return ans;
}

// Initialize a free block with no left or right neighbor given the
// total size the memory points to.
void block_init_island(struct block * const b, const size_t n) {
  block_set_left(b, NULL);
  block_set_freedom(b, 1);
  block_set_size(b, n - sizeof(struct block));
  block_set_end(b, 1);
  // Put block in no freelist
  b->payload[0] = NULL;
  b->payload[1] = NULL;
}





// For non-0 x, the floor of the log_2 of x
int log2floor(const unsigned long long x) {
  return sizeof(unsigned long long)*8 - __builtin_clzll(x) - 1;
}

// Returns the location in the free trie that best fits the number of
// bytes of a block size. Every block in that location will be large
// enough to accomodate an allocation request of size bytes.
struct location size_get_location(const size_t bytes) {
  struct location ans = {0,0};

  // The number of doubewords in the block we need to allocate.
  const size_t dwords = round_up_to_block_size(bytes) >> (word_log - 2);
  
  ans.root = log2floor(((dwords-1) >> word_log)+1);
  ans.leaf = (dwords + (word_bits - 1) - (1ull << (ans.root + word_log))) >> ans.root;

  return ans;
}

void mask_set_bit(size_t * const x, const size_t i, const int b) {
  if (b) {
    *x |= ((size_t)1) << i;
  } else {
    *x &= ~(((size_t)1) << i);
  }
}

int mask_get_bit(const size_t x, const size_t i) {
  return (x >> i) & ((size_t)1);
}

// b must already be free
void tlsf_arena_set_freelist(struct tlsf_arena * const r, struct location const l, struct block * const b) {
  const struct block * const old = r->top[l.root][l.leaf];
  if (old == b) return;
  r->top[l.root][l.leaf] = b;

  // Sets the bits in the trie: 
  if (NULL == b) {
    mask_set_bit(&r->fine[l.root], l.leaf, 0);
    if (0 == r->fine[l.root]) {
      mask_set_bit(&r->coarse, l.root, 0);
    }
  } else if (NULL == old) {
    const size_t old_mask = r->fine[l.root];
    mask_set_bit(&r->fine[l.root], l.leaf, 1);
    if (0 == old_mask) {
      mask_set_bit(&r->coarse, l.root, 1);
    }
  }
}

struct block * tlsf_arena_get_freelist(struct tlsf_arena * const r, struct location const l) {
  return r->top[l.root][l.leaf];
}

void tlsf_arena_detach_block(struct tlsf_arena * const r, struct block * const b) {
  if (b->payload[0]) {
    b->payload[0]->payload[1] = b->payload[1];
    if (b->payload[1]) {
      b->payload[1]->payload[0] = b->payload[0];
    }
  } else {
    const struct location l = size_get_location(block_get_size(b));
    tlsf_arena_set_freelist(r, l, b->payload[1]);
    if (b->payload[1]) {
      b->payload[1]->payload[0] = NULL;
    }
  }
}

void tlsf_arena_add_block(struct tlsf_arena * const r, struct block * const b) {
  struct location l = size_get_location(block_get_size(b));
  b->payload[0] = NULL;
  b->payload[1] = tlsf_arena_get_freelist(r, l);
  if (b->payload[1]) {
    b->payload[1]->payload[0] = b;
  }
  tlsf_arena_set_freelist(r, l, b);
}  


// return l.root >= big_buckets if nothing available
struct location tlsf_arena_find_fitting(const struct tlsf_arena * const r, const size_t n) {
  static const struct location dummy = {big_buckets, 0};
  struct location ans = { big_buckets, 0 };
  size_t size = word_bytes * (n/word_bytes + ((n & (word_bytes - 1)) > 0));
  struct location l = {0, 0};
  if (size < 3*word_bytes) {
    size = 2*word_bytes;
  } else {
    l = size_get_location(size);
    const struct location m = size_get_location(size - word_bytes);
    if (m.leaf == l.leaf) {
      ++l.leaf;
      if (l.leaf == word_bits) {
        l.leaf = 0;
        ++l.root;
        if (l.root == big_buckets) return dummy;
      }
    }
  }
  unsigned long long coarse_shift = r->coarse >> l.root;
  if (0 == coarse_shift) { return dummy; }
  ans.root = l.root + __builtin_ffsll(coarse_shift) - 1;
  const size_t mod_fine = (ans.root == l.root) ? l.leaf : 0;
  const unsigned long long fine_shift = r->fine[ans.root] >> mod_fine;
  if (0 != fine_shift) {
    ans.leaf = mod_fine + __builtin_ffsll(fine_shift) - 1;
  } else {
    coarse_shift = r->coarse >> (l.root+1);
    if (0 == coarse_shift) { return dummy; }
    ans.root = l.root + 1 + __builtin_ffsll(coarse_shift) - 1;
    ans.leaf = __builtin_ffsll(r->fine[ans.root]) - 1;
  }
  return ans;
}


// TODO: what if 0 bytes are requested?
void * tlsf_malloc(struct tlsf_arena * const r, const size_t n) {
  if (NULL == r) return NULL;
  struct location l = tlsf_arena_find_fitting(r, n);
  if (l.root >= big_buckets) {
    return NULL;
  }
  struct block * const b = tlsf_arena_get_freelist(r, l);
  tlsf_arena_detach_block(r, b);
  struct block * const c = block_split_detached(b, n);
  block_set_freedom(b, 0);
  if (NULL != c) {
    tlsf_arena_add_block(r, c);
  }
  assert (block_get_size(b) >= n);
  assert(0 == block_get_freedom(b));
  return b->payload;
}


// TODO: realloc, calloc
// TODO: what is precondition on the size here?
struct tlsf_arena * tlsf_create(char * whole, size_t length) {
  struct tlsf_arena * ans = (struct tlsf_arena *)whole;
  whole += sizeof(struct tlsf_arena);
  length -= sizeof(struct tlsf_arena);

  ans->coarse = 0;
  for (size_t i = 0; i < big_buckets; ++i) {
    ans->fine[i] = 0;
  }
  for (size_t i = 0; i < big_buckets; ++i) {
    for (size_t j = 0; j< word_bits; ++j) {
      ans->top[i][j] = NULL;
    }
  }

  struct block * first = (struct block *)whole;
  whole += sizeof(struct block);
  length -= sizeof(struct block);

  block_init_island(first, sizeof(struct block) * (length/sizeof(struct block)));
  tlsf_arena_add_block(ans, first);
  return ans;
}

/*
struct tlsf_arena * tlsf_create_limited(void * begin, size_t length) {
  return tlsf_create(begin, length);
}
*/

void tlsf_augment(struct tlsf_arena * r, void * begin, size_t length) {
  if (NULL == r) return;
  block_init_island(begin, length);
  tlsf_arena_add_block(r, begin);
}

//const size_t tlsf_padding = sizeof(struct tlsf_arena) + sizeof(struct block);

/*


struct tlsf_arena * tlsf_init_from_malloc(const size_t bsize) {
  if (bsize > max_alloc) return NULL;
  size_t size = word_bytes * (bsize/word_bytes + ((bsize & (word_bytes - 1)) > 0));
  if (size < 2*word_bytes) size = 2*word_bytes;
  const size_t get = size + sizeof(struct tlsf_arena) + sizeof(struct block);
  void * whole = malloc(get);
  return tlsf_init_from_block(whole, get);
}
*/

/*

TOTEST: 
free lists are not circular
(with valgrind) left and right links always work
left links are actually less
stored blocks sum up to the expected size
double links in free lists make sense
*/
  
/* TOCHECK: offsets and alignments */

void tlsf_free(struct tlsf_arena * const r, void * const p) {
  if ((NULL == r) || (NULL == p)) return;
  struct block * b = ((struct block *)p) - 1;

  struct block * const left = block_get_left(b);
  struct block * const right = block_get_right(b);
  if ((NULL != left) && block_get_freedom(left)) {
    tlsf_arena_detach_block(r, left);
    coalesce_detached_blocks(left, b);
    b = left;
  }
  if ((NULL != right) && block_get_freedom(right)) {
    tlsf_arena_detach_block(r, right);
    coalesce_detached_blocks(b, right);
  }

  tlsf_arena_add_block(r, b);
  block_set_freedom(b, 1);
}
