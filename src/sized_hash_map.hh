#ifndef SIZED_HASH_MAP
#define SIZED_HASH_MAP

#include <cstddef>
#include <utility>
#include <cstdlib>
#include <memory>
#include <functional>

#include "node.hh"
#include "deamortized_map.hh"
#include "tlsf_allocator.hpp"

// TODO: use TlsfAllocator for shared_ptrs

template<typename Key, typename Val, typename Hasher, typename Allocator, typename Less>
struct sized_hash_map {

  static typename Allocator::template rebind<Val>::other vallocator;

  const static Hasher hash;

  typedef deamortized_map<Key,Val,Allocator,Less> Bucket;

  typedef typename Bucket::TreeNode DNode;

  Bucket *data;
  size_t slot_count;
  size_t node_count;
  size_t initialized;

  void swap(sized_hash_map& that) {
    swap(this->data, that.data);
    swap(this->slot_count, that.slot_count);
    swap(this->node_count, that.node_count);
    swap(this->initialized, that.initialized);
  }

  static typename Allocator::template rebind<Bucket>::other allocator;

  sized_hash_map(const size_t slot_count) 
  : data(allocator.allocate(slot_count)),
    slot_count(slot_count), 
    node_count(0),
    initialized(0)
  {
    assert (0 == (slot_count & (slot_count - 1)));
    while (initialized < slot_count) initialize_one();
  }

  void initialize_one() {
    assert (initialized < slot_count);
    allocator.construct(&(data[initialized]), Bucket());
    ++initialized;
  }

  void deinitialize_one() {
    assert (initialized > 0);
    if (data[initialized-1].head) {
      data[initialized-1].dealloc_one();
    } else {
      allocator.destroy(&data[initialized-1]);
      --initialized;
    }
  }

  void resize(size_t new_slot_count) {
    assert (0 == initialized);
    allocator.deallocate(data, slot_count);
    data = allocator.allocate(new_slot_count);
    slot_count = new_slot_count;
    node_count = 0;
    initialized = 0;
  }

  ~sized_hash_map() {
    while (initialized > 0) deinitialize_one();
    allocator.deallocate(data, slot_count);
  }
  
  DNode * find(const Key & k) const {
    return data[hash(k) & (slot_count - 1)].find(k);
  }

  std::pair<bool, DNode *> insert(const Key& k, Val* const v) {
    const auto ans = data[hash(k) & (slot_count - 1)].insert;
    if (ans.first) ++node_count;
    return ans;
  }
};


template<typename Key, typename Val, typename Hasher, typename Allocator, typename Less>
const Hasher sized_hash_map<Key,Val,Hasher,Allocator,Less>::hash = Hasher();

template<typename Key, typename Val, typename Hasher, typename Allocator, typename Less>
typename Allocator::template rebind<typename sized_hash_map<Key,Val,Hasher,Allocator,Less>::Bucket>::other sized_hash_map<Key,Val,Hasher,Allocator,Less>::allocator = typename Allocator::template rebind<typename sized_hash_map<Key,Val,Hasher,Allocator,Less>::Bucket>::other();

template<typename Key, typename Val, typename Hasher, typename Allocator, typename Less>
typename Allocator::template rebind<Val>::other sized_hash_map<Key,Val,Hasher,Allocator,Less>::vallocator = typename Allocator::template rebind<Val>::other();

template<typename Key, typename Val, typename Hasher, typename Allocator, typename Less>
void swap(sized_hash_map<Key, Val, Hasher, Allocator, Less>& x, sized_hash_map<Key, Val, Hasher, Allocator, Less>& y) {
  x.swap(y);
}

#endif
