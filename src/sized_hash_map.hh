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

template<typename Key, typename Val, typename Hasher = std::hash<Key>, typename Allocator = TlsfAllocator<char> >
struct sized_hash_map {

  static Hasher hash;

  struct LiveTracker;

  typedef deamortized_map<Key,std::shared_ptr<Val>,LiveTracker,Allocator> Bucket;

  typedef typename Bucket::TreeNode DNode;

  struct LiveTracker {
    DNode *live_prev, *live_next;
    LiveTracker(DNode *live_prev = NULL, DNode *live_next = NULL) :
      live_prev(live_prev), live_next(live_next) {}
  };

  DNode *live_head;
  Bucket *data;
  size_t slot_count;
  size_t node_count;
  size_t tombstone_count;
  size_t initialized;

  static typename Allocator::template rebind<Bucket>::other allocator;
  //static typename Allocator::template rebind<DNode>::other node_allocator;

  sized_hash_map(const size_t slot_count) 
  : live_head(NULL),
    data(allocator.allocate(slot_count)),
    slot_count(slot_count), 
    node_count(0),
    tombstone_count(0),
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
    live_head = NULL;
    data = allocator.allocate(new_slot_count);
    slot_count = new_slot_count;
    node_count = 0;
    tombstone_count = 0;
    initialized = 0;
  }

  ~sized_hash_map() {
    while (initialized > 0) deinitialize_one();
    allocator.deallocate(data, slot_count);
  }
  
  DNode * find(const Key & k) const {
    DNode * ans = data[hash(k) & (slot_count - 1)].find(k);
    if (ans and not ans->val) return NULL;
    return ans;
  }

  bool erase(DNode * c) {
    if (not c or not c->val) return false;
    ++tombstone_count;
    c->val.reset();
    if (live_head == c) live_head = c->live_next;
    if (c->live_prev) c->live_prev->live_next = c->live_next;
    if (c->live_next) c->live_next->live_prev = c->live_prev;
    return true;
  }

  std::pair<bool, DNode *> insert(const Key& k, const std::shared_ptr<Val>& v) {
    auto& slot = data[hash(k) & (slot_count - 1)];
    const auto ans = slot.insert(k, v);
    bool link = false;
    if (ans.first) {
      ++node_count;
      link = true;
    } else if (not ans.second->val) {
      --tombstone_count;
      link = true;
    }
    if (link) {
      ans.second->live_next = live_head;
      if (live_head) live_head->live_prev = ans.second;
      live_head = ans.second;
    }
    return ans;
  }
};


template<typename Key, typename Val, typename Hasher, typename Allocator>
Hasher sized_hash_map<Key,Val,Hasher,Allocator>::hash;

template<typename Key, typename Val, typename Hasher, typename Allocator>
typename Allocator::template rebind<typename sized_hash_map<Key,Val,Hasher,Allocator>::Bucket>::other sized_hash_map<Key,Val,Hasher,Allocator>::allocator;

#endif
