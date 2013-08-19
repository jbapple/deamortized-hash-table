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
     space) is NOT owned by the same memory pool. */
  size_t size; 
  // first item is next in free list
  // need to doubly-link free list to remove items from it in case of coalescing
  struct block * payload[]; // TODO: alignment
};

#define big_buckets (word_bits - 2*word_log + 3)

struct roots {
  //void * end; // why do we need this?
  size_t coarse;
  size_t fine[big_buckets];
  struct block * top[big_buckets][word_bits];
};

/*
void set_size_bit(size_t * const x) {
  *x |= ((size_t)1);
}
*/

void set_ptr_bit(struct block ** x) {
  *x = (struct block *)((size_t)(*x) | ((size_t)1));
}

int get_size_bit(size_t const x) {
  return x & ((size_t)1);
}

int get_ptr_bit(struct block * x) {
  return ((size_t)x & ((size_t)1));
}

void unset_size_bit(size_t * const x) {
  *x &= ~((size_t)1);
}

void unset_ptr_bit(struct block ** x) {
  *x = (struct block *)((size_t)(*x) & (~((size_t)1)));
}

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

void ptr_set_lastbit(struct block ** x, const int f) {
  if (f) {
    *x = (struct block *)((size_t)(*x) | ((size_t)1));
  } else {
    *x = (struct block *)((size_t)(*x) & (~((size_t)1)));
  }
}

void block_set_freedom(struct block * const x, const int f) {
  ptr_set_lastbit(&x->left, f);
}

struct block * get_ptr(struct block * x) {
  return (struct block *)((size_t)(x) & (~((size_t)1)));
}

void mark_free(struct block * const b) {
  set_ptr_bit(&b->left);
}

int check_free(struct block * const b) {
  return get_ptr_bit(b->left);
}

void mark_used(struct block * const b) {
  unset_ptr_bit(&b->left);
}

int check_end(struct block * const b) {
  return 1 - get_size_bit(b->size);
}

void mark_end(struct block * const b) {
  unset_size_bit(&b->size);
}

/*
void mark_not_end(struct block * const b) {
  set_size_bit(&b->size);
}
*/

void set_mask_bit(size_t * const x, const size_t i) {
  *x |= ((size_t)1) << i;
}


/*
void * pop_bigger(struct roots * r, const size_t n) {
  const size_t size = word_bytes * (n/word_bytes + ((n & (word_bytes - 1)) > 0));
  size_t x = word_bits, y = word_bits;
  get_place(size, x, y);
  unsigned long long shift_x = r->coarse << x;
  if (0 == shift_x) { return NULL; }
  int place = x + sizeof(long long)*8 - __builtin_clzll(shift_x);
  const unsigned long long shift_y = r->find[place] << y;
  int here = word_bits;
  if (0 != shift_y) {
    here = y + sizeof(long long)*8 - __builtin_clzll(shift_y);
  } else {
    shift_x = r->coarse << (x+1);
    if (0 == shift_x) { return NULL; }
    place = x + sizeof(long long)*8 - __builtin_clzll(shift_x);
    here = y + sizeof(long long)*8 - __builtin_clzll(r->find[place]);
  }
  struct block * b = r->top[place][place];
  block_set_freedom(b, 0);
  remove_from_list(r, b);
  if (size >= block_get_size(b) + 2*word_bytes + sizeof(struct block)) {
    struct block * const next = b->payload + size/word_bytes;
    next->left = b;
    next->size = block_get_size(b) - size - sizeof(struct block);
    block_set_freedom(next,1);
    if (check_end(b)) { mark_end(next); }
    block_set_size(b, size/word_bytes);
    mark_not_end(b);
  }
  return b->payload;  
}
*/

int get_mask_bit(const size_t x, const size_t i) {
  return (x >> i) & ((size_t)1);
}

void test_roots_setbits(const struct roots * const r) {
  for (size_t i = 0; i < big_buckets; ++i) {
    if (get_mask_bit(r->coarse, i)) {
      assert (r->fine[i] > 0);
      for (size_t j = 0; j < word_bits; ++j) {
        if (get_mask_bit(r->fine[i], j)) {
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


//#define MAX(x,y) ((x) > (y)) ? (x) : (y);

#include <stdio.h>

void get_place(const size_t bytes, size_t * const head, size_t * const tail) {
  /* incorrect: might overflow */
  /* const size_t words = (bytes + sizeof(size_t) - 1)/sizeof(size_t); */
  const size_t dwords = bytes/(word_bytes) + ((bytes & (word_bytes - 1)) > 0);

  if (dwords <= 2) {
    *head = 0;
    *tail = 0;
    return;
  }

  // floor log_2
  const int lg = sizeof(long long)*8 - __builtin_clzll(dwords+word_bits-2) - 1;
  //printf("words: %d, lg floor: %d\n", words, lg);
  if (lg <= (int)word_log) {
    *head = 0;
    *tail = dwords - 2;
  } else {
    *head = lg - word_log;
    *tail = (dwords + word_bits - 2 - (((size_t)1) << (word_log + (*head)))) >> (*head);
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


void place(struct block * const b, struct roots * const r) {
  size_t head, tail;
  get_place(b->size, &head, &tail);
  set_mask_bit(&r->coarse, head);
  set_mask_bit(&r->fine[head], tail); 
  mark_free(b);
  b->payload[0] = NULL;
  b->payload[1] = r->top[head][tail];
  if (r->top[head][tail]) {
    r->top[head][tail]->payload[0] = b;
  }   
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
  const size_t size = 2*word_bytes * (bsize/(2*word_bytes) + ((bsize & ((2*word_bytes) - 1)) > 0));
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

/*

TOTEST: 
free lists are not circular
(with valgrind) left and right links always work
left links are actually less
stored blocks sum up to the expected size
double links in free lists make sense
*/
  

void remove_from_list(struct roots * const r, struct block * const b) {
  if (NULL == b->payload[0]) {
    size_t x = word_bits, y = word_bits;
    get_place(block_get_size(b), &x, &y);
    r->top[x][y] = b->payload[1];
    if (r->top[x][y]) {
      r->top[x][y]->payload[0] = NULL;
    }
  } else {
    b->payload[0]->payload[0] = b->payload[1];
    if (b->payload[1]) {
      b->payload[1]->payload[0] = b->payload[0];
    }
  }
}

/* TOCHECK: offsets and alignments */

void tlsf_free(struct roots * const r, void * const p) {
  struct block * b = ((struct block *)p) - 1;
  mark_free(b);
  struct block * const left = get_ptr(b->left);
  struct block * const right = p + block_get_size(b)/word_bytes;
  if ((NULL != left) && check_free(left)) {
    remove_from_list(r, b);
    block_set_size(left, block_get_size(left) + block_get_size(b) + sizeof(struct block));
    b = left;
  }
  if ((NULL != right) && check_free(right)) {
    block_set_size(b, block_get_size(b) + block_get_size(right) + sizeof(struct block));
    remove_from_list(r, right);
  }
  place(b, r);
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

void test_roots() {
  for (size_t i = 0; i < big_buckets; ++i) {
    for (size_t j = 0; j < word_bits; ++j) {
      struct roots * foo = NULL;
      while (1) {
        size_t many = rword();
        if (many > max_alloc) continue;
        while (NULL == foo) {
          foo = init_tlsf(many);
          many >>= 1;
        }
        break;
      }
      test_roots_setbits(foo);
      test_roots_sizes(foo);
      free(foo);
    }
  }
  struct roots * const foo = init_tlsf(1);
  test_roots_setbits(foo);
  test_roots_sizes(foo);
  free(foo);  
}

int main() {
  printf("%zu\n", sizeof(struct block));
  test_place();
  test_roots();
}

/*
{
  printf("%u\n", word_bits);
  printf("%u\n", sizeof(long long));
  printf("%u\n", sizeof(long));
  printf("%u\n", sizeof(size_t));
}
  
*/
