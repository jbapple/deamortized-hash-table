#include <cstdint>
#include <utility>
#include <cstdlib>
#include <cassert>

using namespace std;

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

template<typename Key, typename Base = lazier_map<Key, BasicBitArray> >
struct quiet_map {
  Base here, there;
  size_t progress;
  
  
  quiet_map() : here(), there(), progress(0) {}

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
