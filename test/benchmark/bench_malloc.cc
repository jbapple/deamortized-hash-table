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
  //char * foo = reinterpret_cast<char *>(malloc(n));
  //foo[n/2] = 'a';
  void * foo = malloc(n);
  free(foo);
}


void bench_malloc_touch(const size_t n) {
  char * foo = reinterpret_cast<char *>(malloc(n));
  foo[n/2] = 'a';
  free(foo);
}

void bench_mmap(const size_t n) {
  void * foo = mmap(NULL, n, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  munmap(foo, n);
}

void bench_new(const size_t n) {
  size_t * foo = new size_t[(n+7)/8]();
  delete foo;
}

int main() {
  auto bar = median_time<bench_new>(100000, 1000, 1000*2*1000);
  for (auto foo : bar) {
    cout << foo.first << '\t' << foo.second[0] << endl;
  }
}
