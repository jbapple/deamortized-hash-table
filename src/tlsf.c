#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define word_bytes sizeof(size_t)
#define word_bits (word_bytes * 8)
// TODO: mock out actual void * and size_t types
#define word_log (3 + (18*word_bytes - word_bytes*word_bytes - 8)/24)

struct block {
  /* left is the address of the free block to the left. We steal the
     last bit to indicate if the block is free.
  */
  struct block * left;
  /* The size in bytes, so, since size is always a word multiple, and
     we assume words are at least two bytes, the last bit is
     free. This bit is set if and only if the next block (in address
     space) is NOT owned by the same memory pool. */
  size_t size; 
  // first item is next in free list
  struct block * payload[]; // TODO: alignment
};

#define big_buckets (word_bits - 2*word_log + 3)

struct roots {
  // TODO: better names than coarse and fine?
  // TODO: lock for multi-threading
  // TODO: make debug code go away when compiled with DNDEBUG
  // TODO: be able to add more memory later
  // TODO: move left counter into previous free block in case sizes support it 
  size_t capacity;
  size_t coarse;
  size_t fine[big_buckets];
  struct block * top[big_buckets][word_bits];
};

size_t tlsf_get_capacity(struct roots * r) { return r->capacity; }

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

size_t block_get_size(const struct block * const x) {
  return (x->size & ~((size_t)1));
}

void block_set_size(struct block * const x, const size_t n) {
  const int old_end = block_get_end(x);
  x->size = n;
  block_set_end(x, old_end);
}

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

struct location {
  uint8_t root, leaf;
};

struct location size_get_location(const size_t bytes) {
  struct location ans = {0,0};
  /* incorrect: might overflow */
  /* const size_t words = (bytes + sizeof(size_t) - 1)/sizeof(size_t); */
  const size_t dwords = bytes/(word_bytes) + ((bytes & (word_bytes - 1)) > 0);

  if (dwords <= 2) {
    return ans;
  }

  // floor log_2
  const int lg = sizeof(long long)*8 - __builtin_clzll(dwords+word_bits-2) - 1;
  //printf("words: %d, lg floor: %d\n", words, lg);
  if (lg <= (int)word_log) {
    //ans.root = 0; // already
    ans.leaf = dwords - 2;
  } else {
    ans.root = lg - word_log;
    ans.leaf = (dwords + word_bits - 2 - (((size_t)1) << lg)) >> ans.root;
  }
  return ans;
}

// TODO: make this "coarse set" and operate on roots
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

// block must already be free
void roots_set_freelist(struct roots * const r, struct location const l, struct block * const b) {
  const struct block * const old = r->top[l.root][l.leaf];
  if (old == b) return;
  r->top[l.root][l.leaf] = b;
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

struct block * roots_get_freelist(struct roots * const r, struct location const l) {
  return r->top[l.root][l.leaf];
}

void roots_detach_block(struct roots * const r, struct block * const b) {
  if (b->payload[0]) {
    b->payload[0]->payload[1] = b->payload[1];
    if (b->payload[1]) {
      b->payload[1]->payload[0] = b->payload[0];
    }
  } else {
    const struct location l = size_get_location(block_get_size(b));
    roots_set_freelist(r, l, b->payload[1]);
    if (b->payload[1]) {
      b->payload[1]->payload[0] = NULL;
    }
  }
}

void roots_add_block(struct roots * const r, struct block * const b) {
  struct location l = size_get_location(block_get_size(b));
  b->payload[0] = NULL;
  b->payload[1] = roots_get_freelist(r, l);
  if (b->payload[1]) {
    b->payload[1]->payload[0] = b;
  }
  roots_set_freelist(r, l, b);
}  

void coalesce_detached_blocks(struct block * const x, struct block * const y) {
  struct block * const z = block_get_right(y);
  if (NULL != z) block_set_left(z, x);
  block_set_size(x, block_get_size(x) + sizeof(struct block) + block_get_size(y));
  block_set_end(x, block_get_end(y));
  assert (block_get_right(x) == z);
  assert (block_get_size(x) > 0);
}

// return l.root >= big_buckets if nothing available
struct location roots_find_fitting(const struct roots * const r, const size_t n) {
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

struct block * block_split_detached(struct block * const b, const size_t n) {
  size_t size = word_bytes * (n/word_bytes + ((n & (word_bytes - 1)) > 0));
  assert (size >= n);
  if (size < 2*word_bytes) size = 2*word_bytes;
  if (block_get_size(b) <= size + sizeof(struct block) + 2*word_bytes) {
    return NULL;
  }

  struct block * const next = (struct block *)(&b->payload[size/word_bytes]);
  assert (next == (struct block *)((char *)(b->payload) + size));
  struct block * const c = block_get_right(b);
  if (NULL != c) block_set_left(c, next);
  next->left = b;
  next->size = block_get_size(b) - size - sizeof(struct block);
  assert (next->size > 0);
  block_set_freedom(next, 1);
  block_set_end(next, block_get_end(b));
  block_set_size(b, size);
  block_set_end(b, 0);
  next->payload[0] = NULL;
  next->payload[1] = NULL;
  assert (block_get_size(next) > 0);
  return next;
}

void block_init(struct block * const b, const size_t n) {
  b->left = NULL;
  b->size = n - sizeof(struct block);
  b->payload[0] = NULL;
  b->payload[1] = NULL;
  block_set_freedom(b, 1);
  block_set_end(b, 1);
}

// TODO: what if 0 bytes are requested?
void * tlsf_malloc(struct roots * const r, const size_t n) {
  if (NULL == r) return NULL;
  struct location l = roots_find_fitting(r, n);
  if (l.root >= big_buckets) {
    const size_t b_size = sizeof(struct block) + ((n < (1 << 20)) ? (1 << 20) : n);
    /* TODO: how can we clear this when we are done with this
       allocator? How can we release it back to the OS? We need
       another piece of information somewhere to help us. */
    struct block * const b = malloc(b_size);
    if (NULL == b) return NULL;
    block_init(b, b_size);
    roots_add_block(r, b);
    l = roots_find_fitting(r, n);
    assert (l.root < big_buckets);
  }
  struct block * const b = roots_get_freelist(r, l);
  roots_detach_block(r, b);
  struct block * const c = block_split_detached(b, n);
  block_set_freedom(b, 0);
  if (NULL != c) {
    roots_add_block(r, c);
  }
  assert (block_get_size(b) >= n);
  assert(0 == block_get_freedom(b));
  return b->payload;
}


// TODO: multithreading

// TODO: realloc, calloc
struct roots * tlsf_init_from_block(void * whole, const size_t get) {
  if (NULL == whole) return NULL;
  struct roots * ans = whole;
  whole += sizeof(struct roots);
  struct block * first = whole;
  whole += sizeof(struct block);
  ans->capacity = get;
  ans->coarse = 0;
  for (size_t i = 0; i < big_buckets; ++i) {
    ans->fine[i] = 0;
  }
  for (size_t i = 0; i < big_buckets; ++i) {
    for (size_t j = 0; j< word_bits; ++j) {
      ans->top[i][j] = NULL;
    }
  }
  block_init(first, sizeof(struct block) * ((get - sizeof(struct roots))/sizeof(struct block)));
  roots_add_block(ans, first);
  return ans;
}

void tlsf_add_block(struct roots * r, void * begin, size_t length) {
  if (NULL == r) return;
  r->capacity += length;
  block_init(begin, length);
  roots_add_block(r, begin);
}

const size_t tlsf_padding = sizeof(struct roots) + sizeof(struct block);

/*


struct roots * tlsf_init_from_malloc(const size_t bsize) {
  if (bsize > max_alloc) return NULL;
  size_t size = word_bytes * (bsize/word_bytes + ((bsize & (word_bytes - 1)) > 0));
  if (size < 2*word_bytes) size = 2*word_bytes;
  const size_t get = size + sizeof(struct roots) + sizeof(struct block);
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

void tlsf_free(struct roots * const r, void * const p) {
  if ((NULL == r) || (NULL == p)) return;
  struct block * b = ((struct block *)p) - 1;

  struct block * const left = block_get_left(b);
  struct block * const right = block_get_right(b);
  if ((NULL != left) && block_get_freedom(left)) {
    roots_detach_block(r, left);
    coalesce_detached_blocks(left, b);
    b = left;
  }
  if ((NULL != right) && block_get_freedom(right)) {
    roots_detach_block(r, right);
    coalesce_detached_blocks(b, right);
  }

  roots_add_block(r, b);
  block_set_freedom(b, 1);
}
