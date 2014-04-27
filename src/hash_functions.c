#include <stdlib.h>
#include <stdint.h>

#if 0
struct hash_state {
  // less than 2^31-1
  uint32_t data;
};

static const uint32_t prime = (((uint32_t)1) << 31)-1;

struct hash_state hash_state_init(uint32_t (*mk_rand)()) {
  struct hash_state ans;
  for (ans.data = ~0; 
       ans.data > prime; 
       ans.data = mk_rand() >> 1);
  return ans;
}

uint32_t mk_coef(const char * const data, const size_t n) {
  union {
    uint32_t coef;
    char parts[4];
  } coef;
  coef.coef = 0;
  for (unsigned i = 0; i < 4 && i < n; ++i) {
    coef.parts[i] = data[i];
  }
  return coef.coef;
}

uint32_t reduce_tiny(const uint64_t x) {
  if (x < prime) {
    return x;
  } else if (x < 2*prime) {
    return x - prime;
  }
  const uint32_t lower = x & prime;
  const uint64_t upper = x >> 31;
  return reduce(reduce(upper) + reduce(lower));
}

uint8_t reduce_tiny(const uint16_t x) {
  static const uint8_t low_prime = (((uint8_t)1) << 7)-1;
  if (x < low_prime) {
    return x;
  } else if (x < 2*low_prime) {
    return x - low_prime;
  }
  const uint8_t lower = x & low_prime;
  const uint16_t upper = x >> 7;
  return reduce_tiny(reduce_tiny(upper) + reduce_tiny(lower));
}

uint32_t hash(const struct hash_state h, const char * const data, const size_t n) {
  uint64_t ans = 0;
  for (size_t i = 0; i < n; i += 4) {
    const uint64_t coef = mk_coef(data + i, n - i);
    ans *= h.data;
    ans = reduce(ans);
    ans += coef;
    ans = reduce(ans);
  }
  return ans;   
}

// Dietzfelbinger's 64 -> 32 bit multiplicative hash on variable length strings
uint32_t hash32(uint64_t a, const char * s) {
  a |= ((uint64_t)1);
  uint64_t accum = 0;
  while (1) {
    accum = (accum >> 32) << 32;
    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
      if (0 == *s) break;
      accum += ((uint64_t)(*s)) << (BITS_PER_BYTE*i);
      ++s;
    }
    accum *= a;
    //accum >>= 32;
    if (0 == *s) return accum >> 32;
  }
}

// Dietzfelbinger's 128 -> 64 bit multiplicative hash on variable length strings
uint64_t hash64(__uint128_t a, const char * s) {
  a |= ((__uint128_t)1);
  __uint128_t accum = 0;
  while (1) {
    accum = (accum >> 64) << 64; // accum &= 0xffffffffffffffff0000000000000000
    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
      if (0 == *s) break;
      accum += ((__uint128_t)(*s)) << (BITS_PER_BYTE*i);
      ++s;
    }
    accum *= a;
    //accum >>= 32;
    if (0 == *s) return accum >> 64;
  }
}



/*
Multilinear of Lemire and Kaser on short strings (<= 63 * 8 bytes)
TODO: reduce multiplications by using multilinear-hm
*/
uint64_t hash64short(const __uint128_t a[64], const char * s) {
  __uint128_t accum = a[0];
  size_t j = 1;
  while (1) {
    __uint128_t raw = 0;
    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
      if (0 == *s) break;
      raw += ((uint64_t)(*s)) << (BITS_PER_BYTE*i);
      ++s;
    }
    accum += raw * a[j];
    if (0 == *s) return accum >> 64;
  }
}

/*
 Dietzfelbinger's 128 -> 64 bit multiplicative hash combined with
Carter & Wegman's iterative halving method for long string hashing
*/
uint64_t hash64long(const __uint128_t a[64], const char * s) {
  uint64_t accum[64];// = {};
  bool full[64] = {};
  while (1) {
    uint64_t raw = 0;
    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
      if (0 == *s) break;
      raw += ((uint64_t)(*s)) << (BITS_PER_BYTE*i);
      ++s;
    }
    for (size_t i = 0; i < 64; ++i) {
      if (full[i]) {
        full[i] = false;
        raw = ((((__uint128_t)accum[i]) + (((__uint128_t)raw) << 64)) * (a[i] | ((__uint128_t)1))) >> 64;
      } else {
        full[i] = true;
        accum[i] = raw;
        break;
      }
    }
    if (0 == *s) break;
  }
  uint64_t ans = 0;
  for (size_t i = 0; i < 64; ++i) {
    if (! full[i]) {
      accum[i] = 0;
    }
    ans = ((((__uint128_t)accum[i]) + (((__uint128_t)ans) << 64)) * (a[i] | ((__uint128_t)1))) >> 64;
  }
  return ans;
}

uint64_t hash64string(const __uint128_t a[64], const char * s) {
  size_t i = 0;
  for(; i <= 63*8; ++i) {
    if (0 == s[i]) break;
  }
  if (i <= 63*8) {
    return ((uint64_t)0x1) | hash64short(a, s);
  } else {
    return (~((uint64_t)0x1)) & hash64long(a, s);
  }
}
#endif

struct wfl_hash_params {
  union {
    __uint128_t data;
    uint64_t words[2];
  } a, b;
};

struct wfl_hash_params wfl_hash_params_init(uint64_t (*mk_rand)()) {
  struct wfl_hash_params ans;
  ans.a.words[0] = mk_rand();
  ans.a.words[1] = mk_rand();
  ans.b.words[0] = mk_rand();
  ans.b.words[1] = mk_rand();
  return ans;
}

uint64_t wfl_hash(const uint64_t x, const struct wfl_hash_params * p) {
  __uint128_t y = x;
  return (y * (p->a.data) + (p->b.data)) >> 64;
}
