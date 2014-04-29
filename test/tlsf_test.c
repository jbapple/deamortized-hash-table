#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include "tlsf-internals.h"

void test_word_sizes() {
  assert (sizeof(void *) == sizeof(size_t));
  assert ((word_bits == 16) || (word_bits == 32) || (word_bits == 64));
  assert ((((size_t)1) << word_log) == word_bits);
  assert (sizeof(struct block) == 2*sizeof(struct block *));
}

//#define max_alloc (word_bytes * (word_bits * ((((size_t)1) << big_buckets) - 1) + 1))
//#define max_alloc (word_bytes * (((1ull << (big_buckets-1))-1) + (((word_bits << (big_buckets-1))-1)/word_bits)))
#define max_alloc (2*word_bytes*(((2*word_bits)<<(big_buckets-1))-word_bits))

void test_roots_setbits(const struct tlsf_arena * const r) {
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
  //*min = word_bytes * ((((((size_t)1) << head) - 1) << word_log) + (((size_t)1) << head) * tail + 2);
  //*max = *min + ((((size_t)1) << head) - 1) * word_bytes;
  *min = 2*word_bytes*(((word_bits + tail)*(1ull << head))-(word_bits-1)) - (2*word_bytes - 1);
  *max = *min + (2*word_bytes - 1) + ((1ull << head) - 1)*2*word_bytes;
}


size_t rword() {
  size_t ans = 0;
  unsigned char * const bytes = (unsigned char *)&ans;
  for (unsigned i = 0; i < word_bytes; ++i) {
    bytes[i] = rand() & 0xff;
  }
  return ans;
}



/*
void test_max_alloc() {
  static const size_t top = max_alloc + word_bits * word_bytes - word_bytes - 1;
  for (size_t i = 0; i < word_bytes; ++i) {
    assert (0xff == ((const unsigned char *)&top)[i]);
  }
}
*/

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
	assert (a + 1 == b);
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
  time_t seed = time(NULL);
  //seed = 1388859547;
  printf("test_place seed is %ld.\n", seed);
  srand(seed);
  //test_max_alloc();
  for (unsigned i = 0; i < word_bits * word_bits; ++i) {
    test_place_limited();
    test_get_place_monotonic();
  }
  test_place_range_involution(word_bits);
  test_place_maximum();
  test_place_range_contiguous();
}


void test_block_alias(const struct block * const a, const struct block * const b) {
  if ((NULL == a) || (NULL == b)) return;
  if (a == b) return;
  if ((size_t)a < (size_t)b) {
    assert ((size_t)(((char *)a) + sizeof(struct block) + block_get_size(a))
            <= (size_t)(b));
  } else {
    assert ((size_t)(a) 
            >= (size_t)(((char *)b) + sizeof(struct block) + block_get_size(b)));
  }
}


void test_roots_sizes(const struct tlsf_arena * const r, const struct block * const b) {
  for (size_t i = 0; i < big_buckets; ++i) {
    for (size_t j = 0; j < word_bits; ++j) {
      struct block * here = r->top[i][j];
      assert ((NULL == here) || (NULL == here->payload[0]));
      size_t begin = 0, end = 0;
      place_range(i, j, &begin, &end);
      while (NULL != here) {
        assert (block_get_size(here) >= begin);
        assert (block_get_size(here) <= end);
        assert ((NULL == here->payload[0]) || (here == here->payload[0]->payload[1]));
        test_block_alias(here, b);
        here = here->payload[1];
      }
    }
  }
}

// returns 0 if we don't know.
// TODO: we don't actually check 0 return properly in test
size_t roots_contiguous_managed_size(const struct tlsf_arena * const r, const struct block * const d) {
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
    any = 1 - block_get_freedom(b);
    assert ((NULL == block_get_left(b)) || (b == block_get_right(block_get_left(b))));
    test_block_alias(b, d);
    b = block_get_left(b);
  }
  any = 0;
  while (NULL != c) {
    ans += block_get_size(c) + sizeof(struct block);
    assert (any | (0 == block_get_freedom(c)));
    any = 1 - block_get_freedom(c);
    assert ((NULL == block_get_right(c)) || (c == block_get_left(block_get_right(c))));
    test_block_alias(c, d);
    c = block_get_right(c);
  }
  return ans;
}


void test_roots_valid(const struct tlsf_arena * const x, const struct block * const b) {
  test_roots_setbits(x);
  test_roots_sizes(x, b);
  roots_contiguous_managed_size(x, b);
}

struct block_counts {
  size_t free_count, used_count;
};


// returns 0 if we don't know.
struct block_counts roots_block_counts(const struct tlsf_arena * const r) {
  struct block_counts ans = {0,0};
  if (0 == r->coarse) return (struct block_counts){0,~0};
  const size_t root = __builtin_ffsll((unsigned long long)r->coarse) - 1;
  const size_t leaf = __builtin_ffsll((unsigned long long)r->fine[root]) - 1;
  struct block * b = r->top[root][leaf];
  struct block * c = block_get_right(b);
  while (NULL != b) {
    if (block_get_freedom(b)) {
      ans.free_count++;
    } else {
      ans.used_count++;
    }
    b = block_get_left(b);
  }
  while (NULL != c) {
    if (block_get_freedom(c)) {
      ans.free_count++;
    } else {
      ans.used_count++;
    }
    c = block_get_left(c);
  }
  assert (ans.used_count + 1 >= ans.free_count);
  return ans;
}

/*
void * allocate(size_t n) {
  return malloc(n);
}

void deallocate(void * p, size_t unused) {
  free(p);
}
*/

void test_roots() {
  const size_t n = sizeof(struct tlsf_arena) + 2*sizeof(struct block) + (1 << 20);
  {
    void * b = malloc(n);
    struct tlsf_arena * const foo = tlsf_create(b, n);
    test_roots_setbits(foo);
    test_roots_sizes(foo, NULL);
    free(b);
  }
  for (size_t i = 0; i < 1000; ++i) {
    void * b = malloc(n);
    struct tlsf_arena * r = tlsf_create(b, n);
    //const size_t total = roots_contiguous_managed_size(r, NULL);
    size_t used = 0;
    const size_t most = 30;
    struct block ** tracked = tlsf_malloc(r, most * sizeof(struct block *));
    for (size_t i = 0; i < most; ++i) {
      tracked[i] = NULL;
    }
    assert (1 == roots_block_counts(r).used_count);
    assert (1 == roots_block_counts(r).free_count);
    //assert (roots_contiguous_managed_size(r, NULL) == total);
    size_t top = 0;
    for (size_t i = 0; i < most; ++i) {
      size_t n = exp(rword() % (size_t)log(most));
      //const struct block_counts before_count = roots_block_counts(r);
      test_roots_valid(r, NULL);
      tracked[top++] = tlsf_malloc(r, n);
      //const struct block_counts after_count = roots_block_counts(r);
      //const size_t actual_total = roots_contiguous_managed_size(r, ((struct block *)tracked[top-1] - 1));
      //assert ((0 == actual_total) || (total == actual_total));
      test_roots_valid(r, NULL);
      if (NULL != tracked[top-1]) {
        used += n;
        //if (after_count.used_count != ~0) {
        //  assert (after_count.used_count == before_count.used_count + 1);
        //  assert (before_count.free_count - after_count.free_count < 2);
        //}
        for (size_t j = 0; j < n; ++j) {
          ((char *)tracked[top-1])[j] = 0xff;
        }
      } else {
        //assert (after_count.used_count == before_count.used_count);
        //assert (after_count.free_count == before_count.free_count);
        // TODO: track how big when malloc fails
        printf("used %zu %zu\n", used, used + n);
        used -= block_get_size((struct block *)(tracked[top-1]) - 1);
        tlsf_free(r,tracked[top-1]);
        //const struct block_counts post_count = roots_block_counts(r);
        //assert (post_count.used_count + 1 == after_count.used_count);
        //assert (post_count.free_count + 1 - after_count.free_count < 3);
        //assert (roots_contiguous_managed_size(r, NULL) == total);
        test_roots_valid(r, NULL);
        if (top > 0) --top;
      }
    }
    while (top > 0) {
      //const struct block_counts before_count = roots_block_counts(r);
      tlsf_free(r, tracked[--top]);
      //const struct block_counts after_count = roots_block_counts(r);
      //assert (after_count.used_count + 1 == before_count.used_count);
      //assert (after_count.free_count + 1 - before_count.free_count < 3);
      //assert (roots_contiguous_managed_size(r, NULL) == total);
      test_roots_valid(r, NULL);
    }
    // TODO: block_count integrated into free and malloc, proper, guarded by NDEBUG
    // TODO: test all is free, here
    free(b);
  }    
}



void test_running_out() {
  const size_t n = sizeof(struct tlsf_arena) + 2*sizeof(struct block) + (1 << 10);
  void * b = malloc(n);
  struct tlsf_arena * const foo = tlsf_create(b, n);
  for (size_t i = 0; i < 15; ++i) {
    (void)tlsf_malloc(foo, 1 << i);
  }
  free(b);
}

int main() {
  test_running_out();
  test_word_sizes();
  test_place();
  test_roots();
}
