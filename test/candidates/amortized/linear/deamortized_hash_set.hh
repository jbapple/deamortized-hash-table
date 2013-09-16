#ifndef DEAMORTIZED_HASH_SET
#define DEAMORTIZED_HASH_SET

#include <cstdint>
#include <functional>
#include <cstdlib>
#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>
#include <iostream>

using namespace std;

using std::vector;
using std::pair;
using std::make_pair;

#include "tlsf_allocator.hpp"

void callout(const size_t many) {
  static size_t top = 0;
  if (many > top) {
    cout << many << endl;
    top = many;
  }
}

template<typename Key,
         typename Hash = std::hash<Key>, 
         typename Equal = std::equal_to<Key>,
         typename Allocate = TlsfAllocator<Key> >
class LimitedHashSet {
  typedef std::size_t size_t;
public:

  LimitedHashSet& operator=(const LimitedHashSet&) = delete;
  LimitedHashSet(const LimitedHashSet&) = delete;
  LimitedHashSet(LimitedHashSet&&) = delete;
  LimitedHashSet& operator=(LimitedHashSet&&) = delete;

public:


  struct Slot {
    bool occupied;
    Key key;
  };
  Hash hasher;
  Equal equaler;
  typedef typename Allocate::template rebind<Slot>::other A;
  A allocator;
  size_t capacity, size;
  Slot * data;
  size_t progress;
  enum ACT : char {
    FILL, CLEAR, INIT
  } state;

public:

  // TODO: overload for std::swap
  // TODO: copy and move and assignment constructors
  void swap(LimitedHashSet & that) {
    std::swap(allocator, that.allocator);
    std::swap(capacity, that.capacity);
    std::swap(size, that.size);
    std::swap(data, that.data);
    std::swap(progress, that.progress);
    std::swap(state, that.state);
  }

  explicit LimitedHashSet(const Allocate& a, const size_t cap)
    : hasher(), equaler(), allocator(a), capacity(cap), size(0), 
    data(allocator.allocate(capacity)), progress(capacity/2), state(FILL)
  {
    for(size_t i = 0; i < capacity; ++i) {
      data[i].occupied = false;
    }
  }

  // TODO: this need to take state into account
  ~LimitedHashSet() {
    switch (state) {
    case FILL:
      for (size_t i = 0; i < capacity; ++i) {
        if (data[i].occupied) allocator.destroy(&data[i]);
      }
      break;
    case CLEAR:
      while (progress < capacity) {
        if (data[progress].occupied) allocator.destroy(&data[progress]);
        ++progress;
      }
      break;
    default: //INIT
      break;
    }
    allocator.deallocate(data, capacity);
  }

  void fill(const size_t left, const LimitedHashSet& that) {
    assert ((FILL == state) or ((INIT == state) and (capacity == progress)));
    if (INIT == state) {
      progress = 0;
      state = FILL;
    }
    // don't care about *my* progress
    const size_t many = left ? ((that.capacity - progress + left - 1)/left) : (that.capacity - progress);
    callout(many);
    for (size_t i = 0; i < many; ++i) {
      if (that.data[progress].occupied) insert(that.data[progress].key);
      ++progress;
    }
  }

  void clear(const size_t left) {
    if (INIT == state) {
      progress = capacity;
      state = CLEAR;
      return;
    }
    if (FILL == state) {
      progress = 0;
      state = CLEAR;
    }
    size_t many = left ? ((capacity - progress + left - 1)/left) : (capacity - progress);
    // ugly hack
    many = std::min((size_t)128,many);
    callout(many);


    for (size_t i = 0; i < many; ++i) {      
      if (data[progress].occupied) allocator.destroy(&data[progress]);
      ++progress;
    }
  }

  void init(const size_t left, const size_t new_capacity) {
    if (FILL == state) return;
    assert ((INIT == state) or ((CLEAR == state) and (capacity == progress)));
    if (CLEAR == state) {
      reset_size(new_capacity);
      progress = 0;
      state = INIT;
    }
    assert (capacity == new_capacity);
    const size_t many = left ? ((capacity - progress + left - 1)/left) : (capacity - progress);
    callout(many);
    for (size_t i = 0; i < many; ++i) {
      data[progress].occupied = false;
      ++progress;
    }
  }

  void reset_size(const size_t new_capacity) {
    if (new_capacity != capacity) {
      allocator.deallocate(data, capacity);
      capacity = new_capacity;
      data = allocator.allocate(capacity);
    }
    size = 0;
  }


  void catch_up(const vector<pair<Key *, pair<size_t, size_t> > > & moved_back) {
    callout(moved_back.size());
    assert (FILL == state);
    for (const auto & kft : moved_back) {
      if ((kft.second.first >= progress) 
          and (kft.second.second < progress)) {
        insert(*kft.first);
      }
    }
  }

private:

  // find a key. capacity must be > 0. if key is not present, returns
  // location where it would be if it were newly placed.
  size_t locate(const Key& k) const {
    const size_t cap_mask = capacity-1;
    auto h = hasher(k) & cap_mask;
    while (true) {
      if (not data[h].occupied)    return h;
      if (equaler(data[h].key, k)) return h;
      h = (h+1) & cap_mask;
    }
  }

  // i < j <= k, mod capacity
  static bool cyclic_between(const size_t& i, const size_t& j, const size_t& k) {
    return (((i < j) and (j <= k))
            or ((i < j) and (k < i))
            or ((j <= k) and (k < i)));
  }
  
public:
  Key* find(const Key& k) {
    if (NULL == data) return nullptr;
    const size_t i = locate(k);
    if (data[i].occupied && equaler(data[i].key, k)) {
      return &data[i].key;
    }
    return nullptr;
  }

public:

  vector<pair<Key *, pair<size_t, size_t> > > erase(const Key& k) {
    vector<pair<Key *, pair<size_t, size_t> > > ans;
    const size_t cap_mask = capacity-1;
    size_t start = locate(k); // the empty location
    if (not data[start].occupied) return ans;
    data[start].occupied = false;
    data[start].key.~Key();
    --size;
    size_t i = (start + 1) & cap_mask;
    while (data[i].occupied) {
      const size_t h = hasher(data[i].key) & cap_mask;
      if (not cyclic_between(start,h,i)) {
        data[start].occupied = true;
        new (&data[start].key) Key(std::move(data[i].key));
        ans.push_back(make_pair(&data[start].key, make_pair(i, start)));
        start = i;
        data[i].occupied = false;
        //data[i].key.~Key(); // no : already moved it?
      }
      i = (i+1) & cap_mask;
    }
    return ans;
  }

  bool insert(const Key& k) {
    assert (size < capacity/2);
    const size_t i = locate(k);
    if (data[i].occupied) return false;
    new (&data[i].key) Key(k);
    ++size;
    data[i].occupied = true;
    return true;
  }

};

// IDEA: store hash values, ones means "empty"

template<typename Key,
         typename Hash = std::hash<Key>, 
         typename Equal = std::equal_to<Key>,
         typename Allocate = TlsfAllocator<Key> >
struct DeamortizedHashSet {

  DeamortizedHashSet() :
    allocator(), far(allocator,0), near(allocator,32) {
  }
        
  Allocate allocator;
  LimitedHashSet<Key, Hash, Equal, Allocate> far, near;
  // 1/4: clear old
  // 1/4: init new
  // 1/2: fill new

  enum ACT : char {
    CLEAR, BIGGER, SMALLER, FILL, SWAP, RELAX
  };

  // TODO: need to distinguish between down clear and up clear
  // how much time you have to do this
  pair<ACT, size_t> get_act_help() const {
    if (near.size < near.capacity/8) return make_pair(SWAP, 1);
    if (near.size < 3*(near.capacity/16)) return make_pair(FILL, near.size - near.capacity/8);
    if (near.size < 7*(near.capacity/32)) return make_pair(SMALLER, near.size - 3*(near.capacity/16));
    if (near.size < near.capacity/4) return make_pair(CLEAR, near.size - 7*(near.capacity/32));
    if (near.size < 5*(near.capacity/16)) return make_pair(CLEAR, 5*(near.capacity/16) - near.size);
    if (near.size < 3*(near.capacity/8)) return make_pair(BIGGER, 3*(near.capacity/8) - near.size);
    if (near.size < near.capacity/2) return make_pair(FILL, near.capacity/2 - near.size);
    return make_pair(SWAP, 1);
  }
  
  pair<ACT, size_t> get_act() const {
    if ((near.capacity <= 32) and (near.size < 8)) return make_pair(RELAX, 0);
    return get_act_help();
  }

  void erase(const Key& k) {
    const vector<pair<Key *, pair<size_t, size_t> > >  moved_back = near.erase(k);
    const auto act = get_act();
    switch (act.first) {
    case RELAX:
      break;
    case CLEAR:
      far.clear(act.second);
      break;
    case BIGGER:
      far.init(act.second, near.capacity*2);
      break;
    case SMALLER:
      far.init(act.second, near.capacity/2);
      break;
    case FILL:
      far.fill(act.second, near);
      far.erase(k);
      far.catch_up(moved_back);
      break;
    default: // SWAP
      far.swap(near);
      far.erase(k);
    }
  }


  bool insert(const Key& k) {
    const bool ans = near.insert(k);
    const auto act = get_act();
    switch (act.first) {
    case RELAX:
      break;
    case CLEAR:
      far.clear(act.second);
      break;
    case BIGGER:
      far.init(act.second, near.capacity*2);
      break;
    case SMALLER:
      far.init(act.second, near.capacity/2);
      break;
    case FILL:
      far.fill(act.second, near);
      far.insert(k);
      break;
    default: // SWAP
      far.swap(near);
      near.insert(k);
    }
    return ans;
  }
};


#endif // LINEAR_HASH_SET
