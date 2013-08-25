/* #if sizeof(void *) == sizeof(unsigned short) */
/* typedef unsigned short word; */
/* #elif sizeof(void *) == sizeof(unsigned int) */
/* typedef unsigned int word; */
/* #elif sizeof(void *) == sizeof(unsigned long) */
/* typedef unsigned long word; */
/* #elif sizeof(void *) == sizeof(unsigned long long) */
/* typedef unsigned long long word; */
/* #else */
/* #error "Can't find word size" */
/* #endif */

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

void test_word_sizes() {
  assert (sizeof(void *) == sizeof(size_t));
  assert ((((size_t)1) << word_log) == word_bits);
}

struct block {
  /* Left is the address of the free block to the left. We steal the
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
  size_t coarse;
  size_t fine[big_buckets];
  struct block * top[big_buckets][word_bits];
};


// TODO: make_block function
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

// returns 0 if we don't know.
size_t roots_contiguous_managed_size(const struct roots * const r) {
  if (0 == r->coarse) return 0;
  const size_t root = __builtin_ffsll((unsigned long long)r->coarse) - 1;
  const size_t leaf = __builtin_ffsll((unsigned long long)r->fine[root]) - 1;
  struct block * b = r->top[root][leaf];
  int any = 1;
  size_t ans = 0;
  struct block * c = block_get_right(b);
  assert ((NULL == c) || (b == block_get_left(c)));
  while (NULL != b) {
    ans += block_get_size(b) + sizeof(struct block);
    assert (any | (0 == block_get_freedom(b)));
    if (1 == block_get_freedom(b)) any = 0;
    assert ((NULL == block_get_left(b)) || (b == block_get_right(block_get_left(b))));
    b = block_get_left(b);
  }
  any = 0;
  while (NULL != c) {
    ans += block_get_size(c) + sizeof(struct block);
    assert (any | (0 == block_get_freedom(c)));
    if (1 == block_get_freedom(c)) any = 0;
    assert ((NULL == block_get_right(c)) || (c == block_get_left(block_get_right(c))));
    c = block_get_right(c);
  }
  return ans;
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
  block_set_size(x, block_get_size(x) + sizeof(struct block) + block_get_size(y));
  block_set_end(x, block_get_end(y));
  assert(block_get_size(x) > 0);
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
  if (size < 2*word_bytes) size = 2*word_bytes;
  if (block_get_size(b) <= size + sizeof(struct block) + 2*word_bytes) {
    return NULL;
  }

  struct block * const next = (struct block *)(&b->payload[size/word_bytes]);
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

void * tlsf_malloc(struct roots * const r, const size_t n) {
  const struct location l = roots_find_fitting(r, n);
  if (l.root >= big_buckets) return NULL;
  struct block * const b = roots_get_freelist(r, l);
  roots_detach_block(r, b);
  struct block * const c = block_split_detached(b, n);
  if (NULL != c) {
    roots_add_block(r, c);
  }
  block_set_freedom(b, 0);
  return b->payload;
}

void test_roots_setbits(const struct roots * const r) {
  for (size_t i = 0; i < big_buckets; ++i) {
    if (mask_get_bit(r->coarse, i)) {
      assert (r->fine[i] > 0);
      for (size_t j = 0; j < word_bits; ++j) {
        if (mask_get_bit(r->fine[i], j)) {
          assert (NULL != r->top[i][j]);
        } else {
          assert (NULL == r->top[i][j]);
        }
      }
    } else {
      assert (0 == r->fine[i]);
    }
  }
}





void place_range(const size_t head, const size_t tail, size_t * const min, size_t * const max) {
  if (0 == head) {
    *min = (tail + 2) * word_bytes;
    *max = *min;
    return;
  }
  *min = word_bytes * ((((((size_t)1) << head) - 1) << word_log) + (((size_t)1) << head) * tail + 2);
  *max = *min + ((((size_t)1) << head) - 1) * word_bytes;
}


size_t rword() {
  size_t ans = 0;
  unsigned char * const bytes = (unsigned char *)&ans;
  for (unsigned i = 0; i < word_bytes; ++i) {
    bytes[i] = rand() & 0xff;
  }
  return ans;
}

const size_t max_alloc = word_bytes * (word_bits * ((((size_t)1) << big_buckets) - 1) + 1);

void test_max_alloc() {
  /*
  const size_t ones = max_alloc - 2;
  const size_t top = max_alloc - 
  for (size_t i = 0; i < word_bytes; ++i) {
    assert (0xff == ((const unsigned char *)&max)[i]);
  }
  */
}

void test_place_limited() {
  const size_t t = rword();
  if (t > max_alloc) test_place_limited();
  const struct location l = size_get_location(t);
  //printf("%#.16zx %zu %zu\n", t, head, tail);
  assert (l.root < big_buckets);
  assert (l.leaf < word_bits);
}

void test_get_place_monotonic() {
  const size_t u = rword();
  const size_t v = u + 1;
  if ((u > max_alloc) || (v > max_alloc)) test_get_place_monotonic();
  const struct location p = size_get_location(u);
  const struct location q = size_get_location(v);
  //printf("%#zx\n", u);
  assert (p.root <= q.root);
  if (p.root < q.root) {
    assert (0 == q.leaf);
    assert (word_bits - 1 == p.leaf);
    assert (p.root + 1 == q.root);
  } else {
    assert ((p.leaf == q.leaf) || (p.leaf + 1 == q.leaf));
  }
}

void test_place_range_contiguous() {
  size_t a = ~0, b = 0, c;
  place_range(0, 0, &c, &a);
  for (size_t i = 0; i < big_buckets; ++i) {
    for (size_t j = 0; j < word_bits; ++j) {
      if (!((i == 0) && (j == 0))) {
	place_range(i, j, &b, &c);
	assert (a + word_bytes == b);
	a = c;
      }
    }
  }
}

void test_place_range_involution(const size_t many) {
  for (size_t i = 0; i < big_buckets; ++i) {
    for (size_t j = 0; j < word_bits; ++j) {
      size_t min, max;
      place_range(i, j, &min, &max);
      assert (max >= min);
      const size_t delta = max - min;
      struct location l;
      if (delta <= many) {
	for (size_t k = min; (k <= max) && (k != 0); ++k) {
	  l = size_get_location(k);
	  //printf("%zu %zu %zu %zu %zu %zu %zu\n",
	  //i, j, min, max, k, head, tail);
	  assert ((l.root == i) && (l.leaf == j));
	}
      } else {
	for (size_t k = 0; k < many; ++k) {
	  const size_t x = min + rword() % delta;
	  l = size_get_location(x);
	  //printf("%zu %zu %zu %zu %zu %zu %zu\n",
	  //i, j, min, max, x, head, tail);
	  assert ((l.root == i) && (l.leaf == j));
	}
      }
    }
  }
}

void test_place_maximum() {
  const size_t t = max_alloc;
  const struct location l = size_get_location(t);
  //printf("%#zx %zu %zu\n", t, head, tail);
  assert (l.root == big_buckets - 1);
  assert (l.leaf == word_bits - 1);
}

void test_place() {
  const time_t seed = time(NULL);
  printf("test_place seed is %ld.\n", seed);
  srand(seed);
  test_max_alloc();
  for (unsigned i = 0; i < word_bits * word_bits; ++i) {
    test_place_limited();
    test_get_place_monotonic();
  }
  test_place_range_involution(word_bits);
  test_place_maximum();
  test_place_range_contiguous();
}

struct roots * init_tlsf(const size_t bsize) {
  if (bsize > max_alloc) return NULL;
  size_t size = word_bytes * (bsize/word_bytes + ((bsize & (word_bytes - 1)) > 0));
  if (size < 2*word_bytes) size = 2*word_bytes;
  const size_t get = size + sizeof(struct roots) + sizeof(struct block);
  void * whole = malloc(get);
  if (NULL == whole) return NULL;
  struct roots * ans = whole;
  whole += sizeof(struct roots);
  struct block * first = whole;
  whole += sizeof(struct block);
  ans->coarse = 0;
  for (size_t i = 0; i < big_buckets; ++i) {
    ans->fine[i] = 0;
  }
  for (size_t i = 0; i < big_buckets; ++i) {
    for (size_t j = 0; j< word_bits; ++j) {
      ans->top[i][j] = NULL;
    }
  }
  first->left = NULL;
  first->size = size;
  first->payload[0] = NULL;
  first->payload[1] = NULL;
  block_set_freedom(first, 1);
  block_set_end(first, 1);
  roots_add_block(ans, first);
  //place(first, ans);
  return ans;
}

#include <stdio.h>

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
  struct block * b = ((struct block *)p) - 1;
  block_set_freedom(b, 1);
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
}




void test_roots_sizes(const struct roots * const r) {
  for (size_t i = 0; i < big_buckets; ++i) {
    for (size_t j = 0; j < word_bits; ++j) {
      struct block * here = r->top[i][j];
      size_t begin = 0, end = 0;
      place_range(i, j, &begin, &end);
      while (NULL != here) {
        assert (block_get_size(here) >= begin);
        assert (block_get_size(here) <= end);
        here = here->payload[0];
      }
    }
  }
}

void test_roots_valid(const struct roots * const x) {
  test_roots_setbits(x);
  test_roots_sizes(x);
}

void test_roots() {
  {
    struct roots * const foo = init_tlsf(1);
    test_roots_setbits(foo);
    test_roots_sizes(foo);
    free(foo);  
  }
  for (size_t i = 0; i < 10000; ++i) {
    struct roots * r = NULL;
    size_t many = rword();
    while (NULL == r) {
      many >>= 1;
      r = init_tlsf(many);
    }
    free(r);
    many = sqrt(many);
    r = init_tlsf(many);
    printf("many: %zu\n", many);
    const size_t total = roots_contiguous_managed_size(r);
    size_t used = 0;
    const size_t most = sqrt(many);
    struct block ** tracked = tlsf_malloc(r, most * sizeof(struct block *));
    for (size_t i = 0; i < most; ++i) {
      tracked[i] = NULL;
    }
    assert (roots_contiguous_managed_size(r) == total);
    size_t top = 0;
    for (size_t i = 0; i < most; ++i) {
      size_t n = exp(rword() % (size_t)log(most));
      tracked[top++] = tlsf_malloc(r, n);
      if (NULL != tracked[top-1]) {
        used += n;
      }
      assert (roots_contiguous_managed_size(r) == total);
      test_roots_valid(r);
      if (NULL == tracked[top-1]) {
        // TODO: track how big when malloc fails
        printf("used %zu %zu\n", used, used + n);
        used -= block_get_size((struct block *)(tracked[top-1]) - 1);
        tlsf_free(r,tracked[top-1]);
        assert (roots_contiguous_managed_size(r) == total);
        test_roots_valid(r);
        if (top > 0) --top;
      }
    }
    while (top > 0) {
      tlsf_free(r, tracked[--top]);
      assert (roots_contiguous_managed_size(r) == total);
      test_roots_valid(r);
    }
    // TODO: test all is free, here
    free(r);
  }    
}

int main() {
  test_word_sizes();
  test_place();
  test_roots();
}
