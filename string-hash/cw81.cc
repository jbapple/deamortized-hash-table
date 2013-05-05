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

struct hasher {
  __uint128_t a;
  uint64_t b;
  hasher() {
    // TODO: better randomness
    const int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) exit(1);
    for (ssize_t fin = 0; fin < 24; fin += read(fd, reinterpret_cast<char*>(this)+fin, 24-fin)) {
      cerr << this << '\t' << fin << endl;
    }
    cerr << endl;
    if (close(fd) < 0) exit(2);
  }
  uint64_t operator()(const __uint128_t & x) const {
    return (a * x + b) >> 64;
  }
};

uint64_t hash_string(const char * in) {
  static hasher hash[64];
  unsigned max_set = 0;
  bool full[64] = {false};
  __uint128_t accum[64] = {0};
  uint64_t data;
  while ((data = stream_read(&in)) > 0) {
    for (unsigned i = 0; i < 64; ++i) {
      uint64_t * buffer = reinterpret_cast<uint64_t*>(&(accum[i]));
      if (not full[i]) {
        full[i] = true;
        max_set = std::max<unsigned>(i, max_set);
        buffer[0] = data;
        break;
      }
      full[i] = false;
      buffer[1] = data;
      data = hash[i](accum[i]);
    }
  }
  //return *reinterpret_cast<uint64_t*>(&(accum[max_set]));
  return hash[max_set](accum[max_set]);
}

int main() {
  std::string hash_me;
  while (cin >> hash_me) 
    cout << hash_string(hash_me.c_str()) << endl;
}
