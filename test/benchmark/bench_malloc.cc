#include <cstdlib>
#include <iostream>

using namespace std;

#include <sys/mman.h>

#include "util.hh"


void bench_calloc(const size_t n) {
  void * foo = calloc(n,1);
  free(foo);
}

void bench_malloc(const size_t n) {
  static const size_t many = 4;
  static void* all[many];
  static size_t last = 0;
  free(all[last]);
  all[last] = malloc(n);
  last = (last+1) & (many-1);
  /*
  static const size_t many = 64;
  void* all[many];
  for (size_t i = 0; i < many; ++i) {
    all[i] = malloc(n);
  }
  for (size_t i = 0; i < many; ++i) {
    free(all[i]);
  }
  */
}


void bench_malloc_touch(const size_t n) {
  char * foo = reinterpret_cast<char *>(malloc(n));
  for (size_t i = 0; i < n; i += 4096) {
    foo[i] += 'a';
  }
  free(foo);
}

void bench_mmap(const size_t n) {
  static const size_t many = 4;
  static void* all[many];
  static size_t sizes[many] = {};
  static size_t last = 0;
  munmap(all[last], sizes[last]);
  all[last] = mmap(NULL, n, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  sizes[last] = n;
  last = (last+1) & (many-1);
  /*
  static const size_t many = 64;
  void* all[many];
  for (size_t i = 0; i < many; ++i) {
    all[i] = mmap(NULL, n, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  }
  for (size_t i = 0; i < many; ++i) {
    munmap(all[i], n);
  }
  */
  //  void * foo = mmap(NULL, n, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  //munmap(foo, n);
}

void bench_new(const size_t n) {
  size_t * foo = new size_t[(n+7)/8]();
  delete foo;
}

int main() {
  auto bar = median_time<bench_malloc>(10000000, 1000, 10000*2*1000);
  for (auto foo : bar) {
    cout << foo.first << '\t' << foo.second[0] << endl;
  }
}
