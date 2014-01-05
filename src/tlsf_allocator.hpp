#ifndef TLSF_ALLOCATOR_HPP
#define TLSF_ALLOCATOR_HPP

#include <atomic>
#include <cstdlib>
#include <mutex>

extern "C" {
#include "tlsf.h"
}


extern std::mutex pool_mutex;

struct TlsfWrapper {
  static void * allocate_fp(size_t n) { return malloc(n); };
  static void deallocate_fp(void * p, size_t) { free(p); }

  roots * tlsf_alloc_pool;
  TlsfWrapper() : tlsf_alloc_pool(tlsf_create(allocate_fp, deallocate_fp)) {}
  ~TlsfWrapper() {
    tlsf_destroy(tlsf_alloc_pool);
  }
};

extern TlsfWrapper tlsf_wrapper;


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

  inline pointer allocate(size_type count) const {
    std::lock_guard<std::mutex> lock(pool_mutex);
    return reinterpret_cast<pointer>(tlsf_malloc(tlsf_wrapper.tlsf_alloc_pool, count * sizeof(T)));
  }
  inline void deallocate(pointer p, size_type) const { 
    std::lock_guard<std::mutex> lock(pool_mutex);
    tlsf_free(tlsf_wrapper.tlsf_alloc_pool, p);
  }

  //    size
  // TODO: overestiamte: consider roots size and block header size
  inline size_type max_size() const { return -1; }


  //    construction/destruction
  inline void construct(pointer p, const T& t) const { new(p) T(t); }
  inline void destroy(pointer p) const { p->~T(); }

  //inline bool operator==(TlsfAllocator const& that) { return this->pool == that.pool; }
  //inline bool operator!=(TlsfAllocator const& a) { return !operator==(a); }
};

#endif
