#include <cstdint>
#include <utility>
#include <cstdlib>
#include <cassert>

using namespace std;

size_t hashf(int x) {return static_cast<size_t>(x);}

template<typename Key>
struct lazy_map {
  struct slot {
    Key key;
    bool occupied;
  };
  slot * data;
  size_t capacity, full;
  lazy_map()
    : data(0), capacity(0), full(0) {}
  lazy_map(const size_t capacity)
    // use malloc to allow Key type without default constructor
    : data(reinterpret_cast<slot*>(malloc(sizeof(slot) * capacity))), 
      capacity(capacity), full(0)
  {
    assert (0 != data);
    for (size_t i = 0; i < capacity; ++i) {
      data[i].occupied = false;
    }
  }

  lazy_map(const lazy_map&) = delete;
  lazy_map& operator=(const lazy_map&) = delete;
  
  // find a key in a nonempty table
  size_t locate(const Key& k) {
    assert (nullptr != data); assert (capacity > 0); assert (0 == (capacity & capacity-1));
    const auto cap_mask = capacity-1;
    auto h = hashf(k) & (cap_mask);
    while (true) {
      if (not data[h].occupied) return h;
      if (data[h].key == k) return h;
      h = (h+1) & cap_mask;
    }
  }

  // place a key
  size_t place(const Key& k) {
    assert (full < capacity);
    const auto i = locate(k);
    if (data[i].occupied) return static_cast<size_t>(-1);
    data[i].occupied = true;
    data[i].key = k;
    ++full;
    return i;
  }

  // i < j <= k, mod capacity
  bool cyclic_between(const size_t& i, const size_t& j, const size_t& k) const {
    return (((i < j) and (j <= k))
            or ((i < j) and (k < i))
            or ((j <= k) and (k < i)));
  }

  void displace(const Key& k, lazy_map * that, const size_t limit) {
    auto start = locate(k);
    if (not data[start].occupied) return;
    data[start].occupied = false;
    const size_t ans = start;
    --full;
    auto i = (start + 1) & (capacity - 1);
    while (data[i].occupied) {
      const auto h = hashf(data[i].key) & (capacity - 1);
      if (not cyclic_between(start,h,i)) {
        data[start].occupied = true;
        data[start].key = data[i].key;
        start = i;
        data[i].occupied = false;
        if (that and (start < limit) and i >= limit) that->place(data[start].key);
      }
      i = (i+1) & (capacity - 1);
    }
  }
  
  void swap(lazy_map * that) {
    std::swap(data, that->data);
    std::swap(capacity, that->capacity);
    std::swap(full, that->full);
  }

  ~lazy_map() {
    if (0 != data) {
      free(data);
      data = 0;
    }
  }

};


template<typename Key>
struct quiet_map {
  lazy_map<Key> here, there;
  size_t progress;
  
  
  quiet_map() : here(), there(), progress(0) {}

  void insert(const Key& k) {
    if (0 == here.capacity) {
      lazy_map<Key> temp(16);
      here.swap(&temp);
    }
    size_t l = here.place(k);
    if (l < progress) there.place(k);
    if (here.full > (here.capacity * 3)/8) {
      if (there.capacity < 2*here.capacity) {
        lazy_map<Key> temp(here.capacity *2);
        there.swap(&temp);
      }
      while ((progress < (here.full - (here.capacity * 3)/8)*8)
             and (progress < here.capacity)) {
        if (here.data[progress].occupied) {
          there.place(here.data[progress].key);
        }
        ++progress;
      }
      if (here.full > here.capacity/2) {
        here.swap(&there);
        there.~lazy_map<Key>();
        progress = 0;
      }
    }
    if ((here.full < (here.capacity *3)/16)
        and (here.capacity > 16)) {
      while ((progress < ((here.capacity * 3)/16 - here.full)*16)
             and (progress < here.capacity)) {
        if (here.data[progress].occupied) {
          there.place(here.data[progress].key);
        }
        ++progress;
      }      
    }
  }

};
