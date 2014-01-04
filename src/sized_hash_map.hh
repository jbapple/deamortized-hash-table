#include <cstddef>
#include <utility>
#include <cstdlib>
#include <memory>

#include "node.hh"
#include "deamortized_map.hh"
#include "tlsf_allocator.hpp"

template<typename Key, typename Val, typename Allocator = TlsfAllocator<char> >
struct sized_hash_map {

  typedef deamortized_map<Key,std::shared_ptr<Val>,LiveTracker,Allocator> Bucket;

  struct LiveTracker;

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
  {}

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
    new (this) sized_hash_map(new_slot_count);
  }

  ~sized_hash_map() {
    while (initialized > 0) deinitialize_one();
    allocator.deallocate(data, slot_count);
  }
  
  DNode * find(const Key & k) const {
    return slots[hash(k) & (slot_count - 1)].find(k);
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

  std::pair<bool, DNode *> insert(const Key& k, const Val& v) {
    const auto slot = slots[hash(k) & (slot_count - 1)];
    const auto ans = slot.bucket.insert(k, std::make_shared<Val>(v));
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
      live_head = ans.second;
    }
    return ans;
  }
};

