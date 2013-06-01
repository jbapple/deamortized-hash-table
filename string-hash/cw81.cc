#include <cstdint>
#include <algorithm>
#include <string>
#include <iostream>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

uint64_t stream_read(const char ** in) {  
  uint64_t ans = 0;
  if (nullptr == *in) {
    return ans;
  }
  for (unsigned i = 0; i < 8; ++i) {
    char * place = reinterpret_cast<char *>(&ans) + i;
    *place = **in;
    if (0 == *place) {
      *in = nullptr;
      return ans;
    }
    ++*in;
  }
  return ans;
}


// TODO: this is Woelfel's construction. What about dietzfelbinger's?
struct hasher {
  __uint128_t a;
  uint64_t b;
  hasher() {
    // TODO: better randomness
    const int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) exit(1);
    for (ssize_t fin = 0; fin < 24; 
         fin += read(fd, reinterpret_cast<char*>(this)+fin, 24-fin)) {
      cerr << this << '\t' << fin << endl;
    }
    cerr << endl;
    if (close(fd) < 0) exit(2);
    a = a | 1; // a must be odd
  }
  uint64_t operator()(const __uint128_t & x) const {
    return (a * x + b) >> 64;
  }
};

struct pt_hasher {
  __uint128_t a0, a1, b;
  pt_hasher() {
    // TODO: better randomness
    const int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) exit(1);
    for (ssize_t fin = 0; fin < static_cast<ssize_t>(sizeof(*this)); 
         fin += read(fd, reinterpret_cast<char*>(this)+fin, sizeof(*this)-fin)) {
      //cerr << this << '\t' << fin << endl;
    }
    //cerr << endl;
    if (close(fd) < 0) exit(2);
    
  }
  uint64_t operator()(const __uint128_t & x) const {
    const uint64_t * y = reinterpret_cast<const uint64_t *>(&x);
    return ((a0+y[0]) * (a1+y[1]) + b) >> 64;
  }
};

struct big_pt_hasher {
  pt_hasher a, b;
  big_pt_hasher() : a(), b() {}
  __uint128_t operator()(const __uint128_t & x, const __uint128_t & y) {
    __uint128_t ans;
    uint64_t * buffer = reinterpret_cast<uint64_t *>(ans);
    buffer[0] = a(x);
    buffer[1] = b(y
  }
};

// TODO: measure in-length first, to mimic sha1 API
uint64_t hash_string(const char * in) {
  // TODO: lazy initialization
  static pt_hasher hash[64];
  unsigned max_set = 0;
  bool full[64] = {false};
  __uint128_t accum[64] = {0};
  uint64_t data;
  while ((data = stream_read(&in)) > 0) {
    for (unsigned i = 0; i < 64; ++i) {
      // TODO: faster to mask or alias?
      uint64_t * buffer = reinterpret_cast<uint64_t*>(&(accum[i]));
      buffer[full[i] ? 1 : 0] = data;
      full[i] = not full[i];
      if (full[i]) {
        max_set = std::max<unsigned>(i, max_set);
        break;
      } else {
        data = hash[i](accum[i]);
      }
    }
  }
  // TODO: bigger output - no need to hash down here necessarily
  return hash[max_set](accum[max_set]);
}

int main() {
  std::string hash_me;
  while (cin >> hash_me) 
    cout << hash_string(hash_me.c_str()) << endl;
}
