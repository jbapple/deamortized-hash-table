#ifndef TLSF_ALLOCATOR_HPP
#define TLSF_ALLOCATOR_HPP

extern "C" {
#include "tlsf.h"
}

// TODO: alignment
template<typename T>
struct TlsfAllocator {
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef std::size_t size_type;
  //typedef std::ptrdiff_t difference_type;
  template<typename U>
  struct rebind {
    typedef TlsfAllocator<U> other;
  };
  
  roots * pool;
  static const size_t start_size = static_cast<size_t>(1) << 34;
  inline explicit TlsfAllocator() : pool(init_tlsf_from_malloc(start_size)) {
    size_t size = start_size >> 1;
    while (NULL == pool) {
      pool = init_tlsf_from_malloc(size);
      size >>= 1;
    }
  }
  // TODO: when to delete pool?
  //inline ~Allocator() {}
  inline TlsfAllocator(TlsfAllocator const& that) : pool(that.pool) {}
  template<typename U>
  inline TlsfAllocator(TlsfAllocator<U> const& that) : pool(that.pool) {}

  //    address
  //inline pointer address(reference r) { return &r; }
  //inline const_pointer address(const_reference r) { return &r; }

  inline pointer allocate(size_type cnt) { 
                                                             //                          typename std::allocator<void>::const_pointer = 0) { 
    return reinterpret_cast<pointer>(tlsf_malloc(pool, cnt * sizeof (T))); 
  }
  inline void deallocate(pointer p, size_type) { 
    tlsf_free(pool, p);
  }

  //    size
  // TODO: overestiamte: consider roots size and block header size
  inline size_type max_size() const { 
    return start_size / sizeof(T);
  }

  //    construction/destruction
  inline void construct(pointer p, const T& t) { new(p) T(t); }
  inline void destroy(pointer p) { p->~T(); }

  inline bool operator==(TlsfAllocator const& that) { return this->pool == that.pool; }
  inline bool operator!=(TlsfAllocator const& a) { return !operator==(a); }
};

#endif
