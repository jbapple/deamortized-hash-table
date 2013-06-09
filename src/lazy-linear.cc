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

  bool is_occupied(const size_t i) const {
    return data[i].occupied;
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

struct BasicBitArray {
  vector<bool> data;
  BasicBitArray(const size_t n) : data(n, false) {}
  BasicBitArray(const BasicBitArray&) = delete;
  BasicBitArray& operator=(const BasicBitArray&) = delete;
  bool check(const size_t& i) const {
    return data[i];
  }
  void set(const size_t& i) {
    data[i] = true;
  }
  void unset(const size_t& i) {
    data[i] = true;
  }
  void swap(BasicBitArray * that) {
    data.swap(that->data);
  }
};

/*
struct BasicBitArray {
  vector<bool> data;
  BasicBitArray(const size_t n) : data(n, false) {}
  BasicBitArray(const BasicBitArray&) = delete;
  BasicBitArray& operator=(const BasicBitArray&) = delete;
  bool check(const size_t& i) const {
    return data[i];
  }
  void set(const size_t& i) {
    data[i] = true;
  }
  void unset(const size_t& i) {
    data[i] = true;
  }
  void swap(BasicBitArray * that) {
    data.swap(that->data);
  }
};
*/

struct AhoBitArray {
  size_t count;
  size_t* sparse;
  size_t* dense;
  AhoBitArray(const size_t n) 
  : count(0), 
    sparse(reinterpret_cast<size_t*>(malloc(n * sizeof(size_t)))),
    dense(reinterpret_cast<size_t*>(malloc(n * sizeof(size_t)))) {}

  AhoBitArray(const AhoBitArray&) = delete;
  AhoBitArray& operator=(const AhoBitArray&) = delete;
  bool check(const size_t& i) const {
    return ((sparse[i] < count) and (dense[sparse[i]] == i));
  }
  void set(const size_t& i) {
    if (check(i)) return;
    dense[count] = i;
    sparse[i] = count;
    ++count;
  }
  void unset(const size_t& i) {
    if (not check(i)) return;
    dense[sparse[i]] = dense[count-1];
    sparse[dense[count-1]] = i;
    --count;
  }
  void swap(AhoBitArray * that) {
    std::swap(count, that->count);
    std::swap(sparse, that->sparse);
    std::swap(dense, that->dense);
  }
  ~AhoBitArray() {
    free(sparse); free(dense);
  }
};


template<typename Key, typename BitArray>
struct lazier_map {
  struct slot {
    Key key;
  };
  BitArray occupied;
  slot * data;
  size_t capacity, full;
  lazier_map()
    : occupied(0), data(0), capacity(0), full(0) {}
  lazier_map(const size_t capacity)
    // use malloc to allow Key type without default constructor
    : occupied(capacity), data(reinterpret_cast<slot*>(malloc(sizeof(slot) * capacity))), 
      capacity(capacity), full(0)
  {
  }

  lazier_map(const lazier_map&) = delete;
  lazier_map& operator=(const lazier_map&) = delete;
  
  // find a key in a nonempty table
  size_t locate(const Key& k) {
    assert (nullptr != data); assert (capacity > 0); assert (0 == (capacity & capacity-1));
    const auto cap_mask = capacity-1;
    auto h = hashf(k) & (cap_mask);
    while (true) {
      if (not occupied.check(h)) return h;
      if (data[h].key == k) return h;
      h = (h+1) & cap_mask;
    }
  }

  // place a key
  size_t place(const Key& k) {
    assert (full < capacity);
    const auto i = locate(k);
    if (occupied.check(i)) return static_cast<size_t>(-1);
    occupied.set(i);
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

  void displace(const Key& k, lazier_map * that, const size_t limit) {
    auto start = locate(k);
    if (not occupied.check(start)) return;
    occupied[start].unset();
    const size_t ans = start;
    --full;
    auto i = (start + 1) & (capacity - 1);
    while (occupied.check(i)) {
      const auto h = hashf(data[i].key) & (capacity - 1);
      if (not cyclic_between(start,h,i)) {
        occupied.set(start);
        data[start].key = data[i].key;
        start = i;
        occupied.unset(i);
        if (that and (start < limit) and i >= limit) that->place(data[start].key);
      }
      i = (i+1) & (capacity - 1);
    }
  }
  
  void swap(lazier_map * that) {
    std::swap(data, that->data);
    std::swap(capacity, that->capacity);
    std::swap(full, that->full);
    occupied.swap(&that->occupied);
  }

  ~lazier_map() {
    if (0 != data) {
      free(data);
      data = 0;
    }
  }

  bool is_occupied(const size_t i) const {
    return occupied.check(i);
  }

};

template<typename Key, typename Base = lazier_map<Key, AhoBitArray> >
struct quiet_map {
  Base here, there;
  size_t progress;
  
  
  quiet_map() : here(), there(), progress(0) {}

  size_t find(const Key& k) {
    return here.locate(k);
  }

  void insert(const Key& k) {
    if (0 == here.capacity) {
      Base temp(16);
      here.swap(&temp);
    }
    size_t l = here.place(k);
    if (l < progress) there.place(k);
    if (here.full > (here.capacity * 3)/8) {
      if (there.capacity < 2*here.capacity) {
        Base temp(here.capacity *2);
        there.swap(&temp);
      }
      while ((progress < (here.full - (here.capacity * 3)/8)*8)
             and (progress < here.capacity)) {
        if (here.is_occupied(progress)) {
          there.place(here.data[progress].key);
        }
        ++progress;
      }
      if (here.full > here.capacity/2) {
        here.swap(&there);
        Base temp;
        there.swap(&temp);
        progress = 0;
      }
    }
    if ((here.full < (here.capacity *3)/16)
        and (here.capacity > 16)) {
      while ((progress < ((here.capacity * 3)/16 - here.full)*16)
             and (progress < here.capacity)) {
        if (here.is_occupied(progress)) {
          there.place(here.data[progress].key);
        }
        ++progress;
      }      
    }
  }

};
