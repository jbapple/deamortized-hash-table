#ifndef TLSF_ALLOCATOR_HPP
#define TLSF_ALLOCATOR_HPP

#include <atomic>
#include <cstdlib>

extern "C" {
#include "tlsf.h"
}

extern std::atomic<roots *> tlsf_alloc_pool;

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
  
  template<typename U>
  inline TlsfAllocator(TlsfAllocator<U> const&) {}

  inline pointer allocate(size_type count) {
    auto pool = tlsf_alloc_pool.load();
    if (NULL == pool) {
      const size_t size = tlsf_padding + count * sizeof(T);
      void * const block = malloc(size);
      if (NULL == block) return nullptr;
      std::atomic<roots *> new_pool = tlsf_init_from_block(block, size);
      roots * expected = nullptr;
      if (tlsf_alloc_pool.compare_exchange_strong(expected, new_pool)) {
        pool = new_pool;
      } else {
        free(block);
      }
    }
    pointer ans = reinterpret_cast<pointer>(tlsf_malloc(pool, count * sizeof(T)));
    if (NULL == ans) {
      const size_t size = tlsf_get_capacity(pool) + count * sizeof(T);
      void * const block = malloc(size);
      if (NULL == block) return nullptr;
      tlsf_add_block(pool, block, size);
      // TODO: lock around mallc and free in TLSF
    }
    ans = reinterpret_cast<pointer>(tlsf_malloc(pool, count * sizeof(T)));
    if (NULL == ans) return nullptr;
    return ans;
  }
  inline void deallocate(pointer p, size_type) { 
    tlsf_free(tlsf_alloc_pool.load(), p);
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
