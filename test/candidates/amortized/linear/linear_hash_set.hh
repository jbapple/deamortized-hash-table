#ifndef LINEAR_HASH_SET
#define LINEAR_HASH_SET

#include <cstdint>
#include <functional>
#include <cstdlib>
#include <cassert>
#include <cstddef>

template<typename Key,
         typename Hash = std::hash<Key>, 
         typename Equal = std::equal_to<Key> >
class LinearHashSet {
  typedef std::size_t size_t;
public:
  /*
  explicit LinearHashSet(const size_t capacity);
  LinearHashSet();
  ~LinearHashSet();
  bool insert(const Key&);
  void erase(const Key&);
  bool member(const Key&);
  void swap(LinearHashSet&);
  */

  LinearHashSet& operator=(const LinearHashSet&) = delete;
  LinearHashSet(const LinearHashSet&) = delete;

  LinearHashSet& operator=(LinearHashSet&&) = delete;

private:


  // The load (percentage of full slots) is never more than 100/up_limit
  static const size_t up_limit = 2;
  // When the load grows too high, the hash table is resized to up_scale * old_capacity
  static const size_t up_scale = 2;
  // The load (percentage of non-full slots) is never less than 100/down_limit
  static const size_t down_limit = 8;
  // When the load is too low, hash table is resized to down_scale * old_capacity
  static const size_t down_scale = 2;
  // TODO: static assert these numbers are in a good ratio to each other
  
  // TODO: should these be const? be elided in favor of referencing some globals?
  Hash hasher;
  Equal equaler;
  size_t capacity, size;
  struct Slot {
    bool occupied;
    Key key;
  };
  Slot * data;

public:

  LinearHashSet()
    : hasher(), equaler(), capacity(0), size(0), data(NULL)
  {}

  explicit LinearHashSet(const size_t capacity)
    : hasher(), equaler(), capacity(capacity), size(0), 
      data(capacity ? reinterpret_cast<Slot*>(malloc(capacity * sizeof(Slot))) : NULL)
  {
    for (size_t i = 0; i < capacity; ++i) {
      data[i].occupied = false;
    }
  }

  explicit LinearHashSet(LinearHashSet&& that) 
    : hasher(), equaler(), capacity(that.capacity), size(that.size), data(that.data)
  {
    that.capacity = 0;
    that.size = 0;
    that.data = NULL;
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
  void erase(const Key& k) {
    if (size * down_scale < capacity) downsize();
    if (NULL == data) return;
    return nr_erase(k);
  }

private:

  void nr_erase(const Key& k) {
    const size_t cap_mask = capacity-1;
    size_t start = locate(k); // the empty location
    if (not data[start].occupied) return;
    data[start].occupied = false;
    data[start].key.~Key();
    --size;
    size_t i = (start + 1) & cap_mask;
    while (data[i].occupied) {
      const size_t h = hasher(data[i].key) & cap_mask;
      if (not cyclic_between(start,h,i)) {
        data[start].occupied = true;
        new (&data[start].key) Key(std::move(data[i].key));
        start = i;
        data[i].occupied = false;
        data[i].key.~Key();
      }
      i = (i+1) & cap_mask;
    }
  }

public:
  bool insert(const Key& k) {
    if (NULL == data) upsize();
    if (size * up_limit > capacity) upsize();
    return nr_insert(k);
  }

private:
  bool nr_insert(const Key& k) {
    const size_t i = locate(k);
    if (data[i].occupied) return false;
    new (&data[i].key) Key(k);
    return true;
  }
  
private:
  void upsize() {
    const auto new_size = (0 == capacity) ? 1 : (capacity * up_scale);
    resize(new_size);
  }

  void downsize() {
    const auto new_size = (1 == capacity) ? 0 : (capacity / down_scale);
    resize(new_size);
  }

  void resize(const size_t new_size) {
    LinearHashSet* that = new LinearHashSet(new_size);
    for (size_t i = 0; i < capacity; ++i) {
      if (data[i].occupied) {
        that->nr_insert(data[i].key);
      }
    }
    swap(*that);
    delete that;
  }

public:

  // TODO: overload for std::swap
  // TODO: copy and move and assignment constructors
  void swap(LinearHashSet & that) {
    std::swap(capacity, that.capacity);
    std::swap(size, that.size);
    std::swap(data, that.data);
  }


public:
  ~LinearHashSet() {
    if (NULL != data) {
      for (size_t i = 0; i < capacity; ++i) {
        if (data[i].occupied) {
          data[i].key.~Key();
        }
      }
      free(data);
      data = NULL;
      size = 0;
      capacity = 0;
    }
  }
};

#endif // LINEAR_HASH_SET
