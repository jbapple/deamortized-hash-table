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

#define word_bytes sizeof(size_t)
#define word_bits (word_bytes * 8)
// TODO: void * is same size
#define word_log (3 + (18*word_bytes - word_bytes*word_bytes - 8)/24)
// TODO: only works on 32 and 64 bit

struct block {
  /* Left is the address of the free block to the left. We steal the
     last bit to indicate if the block is free.
  */
  struct block * left;
  /* The size in bytes, so, since size is always a word multiple, and
     we assume words are at least two bytes, the last bit is
     free. This bit is set if and only if the next block (in address
     space) is owned by the same memory pool. */
  size_t size; 
  // first item is next in free list
  struct block * payload[]; // TODO: alignment
};

#define big_buckets (word_bits - 2*word_log + 4)

struct roots {
  //void * end; // why do we need this?
  size_t coarse;
  size_t fine[big_buckets];
  struct block * top[big_buckets][word_bits];
};

//#define MAX(x,y) ((x) > (y)) ? (x) : (y);

#include <stdio.h>

void get_place(const size_t bytes, size_t * const head, size_t * const tail) {
  /* incorrect: might overflow */
  /* const size_t words = (bytes + sizeof(size_t) - 1)/sizeof(size_t); */
  const size_t words = bytes/word_bytes + ((bytes & (word_bytes - 1)) > 0);

  if (words < 1) {
    *head = 0;
    *tail = 0;
    return;
  }

  // floor(log_2(words))
  const int lg = sizeof(long long)*8 - __builtin_clzll(words) - 1;
  //printf("words: %d, lg floor: %d\n", words, lg);
  if (lg >= (int)word_log) {
    *head = 1 + lg - word_log;
    *tail = (words - (((size_t)1) << lg)) >> (*head - 1);
  } else {
    *head = 0;
    *tail = words;
  }
}

void place_range(const size_t head, const size_t tail, size_t * const min, size_t * const max) {
  if (0 == head) {
    *min = tail * word_bytes;
    *max = *min;
    return;
  }
  *min = word_bytes * ((((size_t)1) << (word_log + head - 1)) + tail * (((size_t)1) << (head - 1)));
  *max = *min + ((((size_t)1) << (head - 1)) - 1) * word_bytes;
}
#include <stdlib.h>

size_t rword() {
  size_t ans = 0;
  unsigned char * const bytes = (unsigned char *)&ans;
  for (unsigned i = 0; i < word_bytes; ++i) {
    bytes[i] = rand() & 0xff;
  }
  return ans;
}

#include <assert.h>
#include <stdio.h>

const size_t max_alloc = 2*((((size_t)1) << (word_bits - 1)) - (word_bytes/2));

void test_max_alloc() {
  const size_t max = max_alloc + word_bytes - 1;
  for (size_t i = 0; i < word_bytes; ++i) {
    assert (0xff == ((const unsigned char *)&max)[i]);
  }
}

void test_place_limited() {
  size_t head = word_bits, tail = word_bits;
  const size_t t = rword();
  if (t > max_alloc) test_place_limited();
  get_place(t, &head, &tail);
  //printf("%#.16zx %zu %zu\n", t, head, tail);
  assert (head < big_buckets);
  assert (tail < word_bits);
}

void test_get_place_monotonic() {
  size_t u1 = big_buckets, u2 = word_bits, v1 = 0, v2 = 0;
  const size_t u = rword();
  const size_t v = u + 1;
  if ((u > max_alloc) || (v > max_alloc)) test_get_place_monotonic();
  get_place(u, &u1, &u2);
  get_place(v, &v1, &v2);
  //printf("%#zx\n", u);
  assert (u1 <= v1);
  if (u1 < v1) {
    assert (0 == v2);
    assert (word_bits - 1 == u2);
    assert (u1 + 1 == v1);
  } else {
    assert ((u2 == v2) || (u2 + 1 == v2));
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
      size_t head = word_bits, tail = word_bits;
      if (delta <= many) {
	for (size_t k = min; (k <= max) && (k != 0); ++k) {
	  get_place(k, &head, &tail);
	  //printf("%zu %zu %zu %zu %zu %zu %zu\n",
	  //i, j, min, max, k, head, tail);
	  assert ((head == i) && (tail == j));
	}
      } else {
	for (size_t k = 0; k < many; ++k) {
	  const size_t x = min + rword() % delta;
	  get_place(x, &head, &tail);
	  //printf("%zu %zu %zu %zu %zu %zu %zu\n",
	  //i, j, min, max, x, head, tail);
	  assert ((head == i) && (tail == j));
	}
      }
    }
  }
}

void test_place_maximum() {
  size_t head = word_bits, tail = word_bits;
  const size_t t = max_alloc;
  get_place(t, &head, &tail);
  //printf("%#zx %zu %zu\n", t, head, tail);
  assert (head == big_buckets - 1);
  assert (tail == word_bits - 1);
}

#include <time.h>
#include <stdio.h>

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

void set_size_bit(size_t * const x) {
  *x |= ((size_t)1);
}

void set_ptr_bit(struct block ** x) {
  *x = (struct block *)((size_t)(*x) | ((size_t)1));
}

int get_size_bit(size_t const x) {
  return x & ((size_t)1);
}

int get_ptr_bit(void ** x) {
  return ((size_t)(*x) & ((size_t)1));
}

void unset_size_bit(size_t * const x) {
  *x &= ~((size_t)1);
}

void unset_ptr_bit(struct block ** x) {
  *x = (struct block *)((size_t)(*x) & (~((size_t)1)));
}

size_t get_size(size_t const x) {
  return (x & ~((size_t)1));
}

struct block * get_ptr(struct block * x) {
  return (struct block *)((size_t)(x) & (~((size_t)1)));
}

void mark_free(struct block * const b) {
  set_ptr_bit(&b->left);
}

void mark_used(struct block * const b) {
  unset_ptr_bit(&b->left);
}

void mark_end(struct block * const b) {
  unset_size_bit(&b->size);
}

void mark_not_end(struct block * const b) {
  set_size_bit(&b->size);
}

void set_mask_bit(size_t * const x, const size_t i) {
  *x |= ((size_t)1) << i;
}

void place(struct block * const b, struct roots * const r) {
  size_t head, tail;
  get_place(b->size, &head, &tail);
  set_mask_bit(&r->coarse, head);
  set_mask_bit(&r->fine[head], tail);
  b->payload[0] = r->top[head][tail];
  mark_free(b);
  r->top[head][tail] = b;
}


/*
void tlsf_free(void * const p) {
   
}

struct block * const displace(struct roots * const r, const size_t head, const size_t tail) {
  struct block * ans = r->top[head][tail];
  r->top[head][tail] = ans->prev;
}
*/

struct roots * init_tlsf(const size_t bsize) {
  const size_t size = word_bytes * (bsize/word_bytes + ((bsize & (word_bytes - 1)) > 0));
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
  mark_free(first);
  mark_end(first);
  place(first, ans);
  return ans;
}

#include <stdio.h>

int main() {
  printf("%zu\n", sizeof(struct block));
  test_place();
  struct roots * foo = init_tlsf(1);
  free(foo);
}

/*
{
  printf("%u\n", word_bits);
  printf("%u\n", sizeof(long long));
  printf("%u\n", sizeof(long));
  printf("%u\n", sizeof(size_t));
}
  
*/
