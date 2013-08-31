#define _GNU_SOURCE // for RTLD_NEXT
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>

#include "tlsf.c"

struct roots * pool = NULL;

char basic[1ull << 20];
struct roots * basic_pool = NULL;

int begin = 0;
int in_printf = 0;

void *malloc(size_t size) {
  //  static size_t recursion = 0;
  //++recursion;
  //puts("malloc");
  //static const char * digits = "0123456789";
  //puts(digits  + ((recursion / 100) % 10));
  //puts(digits  + ((recursion / 10) % 10));
  //puts(digits  + (recursion % 10));
  //printf("malloc \n");

  if (NULL == pool) {
    if (begin) {
      basic_pool = init_tlsf_from_block(basic, 1ull << 20);
      return tlsf_malloc(basic_pool, size);
    }

    static const size_t heap_size = 1ull << 30;
    ++begin;
    void * loc = mmap(NULL, heap_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    --begin;
    pool = init_tlsf_from_block(loc, heap_size);
    assert (NULL != pool);
    /*
    (void) dlerror();
    puts("dlsym");
    void * (*funcp)(size_t) = NULL;
    *(void**) (&funcp) = dlsym(RTLD_NEXT, "malloc");
    puts("dlsymmed");
    if (NULL == funcp) { printf("failed dlsym\n"); exit(1); }
    const char * const err = dlerror();
    if ( err != NULL) { printf("failed dlsym: %s", err); exit(1); }
    puts("dlsymmed successfull");
    if (NULL == pool) {
      void * loc = funcp(heap_size);
      //void * loc = mmap(NULL, heap_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
      //printf("mmapped\n");
      pool = init_tlsf_from_block(loc, heap_size);
    }
    assert (NULL != pool);
    */
  }

  //--recursion;
  void * ans = tlsf_malloc(pool, size);
  if (! in_printf) {
    ++in_printf;
    //printf("malloc\t %zu\t %#zx\n", size, (size_t)ans);
    --in_printf;
  }
  return ans;
}

// TODO: check where we are, free, realloc, calloc, malloc in accordance

void free(void * x) {
  
  if (begin) {
    tlsf_free(basic_pool, x);
    return;
  }
  assert (NULL != pool);
  if (! in_printf) {
    ++in_printf;
    //printf("free\t %#zx\n", (size_t)x);
    --in_printf;
  }    
  tlsf_free(pool, x);
}

void *calloc(size_t nmemb, size_t size) {
  puts("calloc");
  void * ans = malloc(nmemb * size);
  for (size_t i = 0; i < nmemb * size; ++i) {
    ((char *) ans)[i] = 0;
  }
  return ans;
}

#define MIN(x,y) (((x) < (y)) ? (x) : (y))

void *realloc(void *ptr, size_t size) {
  puts("realloc");
  if (NULL == ptr) return malloc(size);
  if (0 == size) {
    free(ptr);
    return NULL;
  }
  void * ans = malloc(size);
  for (size_t i = 0; i < MIN(size, block_get_size(((struct block *)ptr)-1)); ++i) {
    ((char *)ans)[i] = ((char *) ptr)[i];
  }
  free(ptr);
  return ans;
}


/*
int main() {
  malloc(1);
}
*/
