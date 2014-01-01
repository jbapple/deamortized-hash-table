#ifndef TLSF_ALLOCATOR_HPP
#define TLSF_ALLOCATOR_HPP

#include <atomic>
#include <cstdlib>
#include <mutex>

extern "C" {
#include "tlsf.h"
}

extern roots * tlsf_alloc_pool;
extern std::mutex pool_mutex;


// TODO: alignment
// TODO: thread safety
// TODO: thread local malloc. PROBLEM: can't pass blocks from thread to thread
// TODO: global malloc lock
template<typename T>
struct TlsfAllocator {
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef std::size_t size_type;
  template<typename U>
  struct rebind {
    typedef TlsfAllocator<U> other;
  };
  
  TlsfAllocator() {}

  template<typename U>
  inline TlsfAllocator(TlsfAllocator<U> const&) {}

  inline pointer allocate(size_type count) {
    std::lock_guard<std::mutex> lock(pool_mutex);
    if (NULL == tlsf_alloc_pool) {
      const size_t size = tlsf_padding + count * sizeof(T);
      void * const block = malloc(size);
      if (NULL == block) return nullptr;
      tlsf_alloc_pool = tlsf_init_from_block(block, size);
    }
    pointer ans = reinterpret_cast<pointer>(tlsf_malloc(tlsf_alloc_pool, count * sizeof(T)));
    if (NULL == ans) {
      const size_t size = tlsf_get_capacity(tlsf_alloc_pool) + count * sizeof(T);
      void * const block = malloc(size);
      if (NULL == block) return nullptr;
      tlsf_add_block(tlsf_alloc_pool, block, size);
    }
    ans = reinterpret_cast<pointer>(tlsf_malloc(tlsf_alloc_pool, count * sizeof(T)));
    if (NULL == ans) return nullptr;
    return ans;
  }
  inline void deallocate(pointer p, size_type) { 
    std::lock_guard<std::mutex> lock(pool_mutex);
    tlsf_free(tlsf_alloc_pool, p);
  }

  //    size
  // TODO: overestiamte: consider roots size and block header size
  inline size_type max_size() const { return -1; }


  //    construction/destruction
  inline void construct(pointer p, const T& t) { new(p) T(t); }
  inline void destroy(pointer p) { p->~T(); }

  inline bool operator==(TlsfAllocator const& that) { return this->pool == that.pool; }
  inline bool operator!=(TlsfAllocator const& a) { return !operator==(a); }
};

#endif
