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
  LinearHashSet(const size_t capacity = start_size);
  ~LinearHashSet();
  void swap(LinearHashSet&);
  pair<iterator, bool> cover(const Key&);
  
  //const Key* find(const Key&) const; // returns nullptr if not found
  //Key* find(const Key&); // returns nullptr if not found
  //void erase(Key *);

  LinearHashSet& operator=(const LinearHashSet&) = delete;
  LinearHashSet(const LinearHashSet&) = delete;
  LinearHashSet(LinearHashSet&&) = delete;
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
  // The percentage of tombstoned slots is never more than 100/deleted_limit
  static const size_t tombstoned_limit = 3;
  // empty hash tables start with this size
  static const size_t start_size = 0;
  
  Hash hasher;
  Equal equaler;
  size_t capacity, full_count, tombstoned_count;
  struct Slot {
    bool occupied;
    Key key;
  };
  Slot * data;

public:

  LinearHashSet(const size_t capacity = start_size)
    : hasher(), equaler(), capacity(capacity), full_count(0), tombstoned_count(0),
      data( (0 == capacity) ? NULL : 
            reinterpret_cast<Slot*>(malloc(sizeof(Slot) * capacity))) 
  {
    for (size_t i = 0; i < capacity; ++i) {
      data[i].status = Status::EMPTY;
    }
  }

  struct InsertResult {
    enum class Presence {
      ABSENT, PRESENT
    } presence;
    Slot* location;
  };

private:

  // find a key. capacity must be > 0. if key is not present, returns
  // location where it would be if it were newly placed.
  size_t locate(const Key& k) const {
    const auto cap_mask = capacity-1;
    auto h = hasher(k) & cap_mask;
    while (true) {
      if (data[h].status != Status::FULL) return h; // TODO: this is incorrec twhen searching for a key that is in the table
      if (equaler(data[h].key, k))        return h;
      h = (h+1) & cap_mask;
    }
  }

  // i < j <= k, mod capacity
  static bool cyclic_between(const size_t& i, const size_t& j, const size_t& k) {
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
    while (data.data[i].occupied) {
      const auto h = hasher(data[i].key) & (capacity - 1);
      if (not cyclic_between(start,h,i)) {
        data.data[start].occupied = true;
        data.data[start].key = data[i].key;
        start = i;
        data.data[i].occupied = false;
      }
      i = (i+1) & (capacity - 1);
    }
  }


  // place a key in a non-empty table
  InsertResult place(const Key& k) {
    Slot& s = data[locate(k)];
    InsertResult ans = { InsertResult::Presence::ABSENT, &s };
    switch (s.status) {
    case Status::TOMBSTONED:
      --tombstoned_count;
      // fall through:
    case Status::EMPTY:
      s.status = Status::FULL;
      s.key.~Key();
      new (&s.key) Key(k);
      ++full_count;
      return ans;
    case Status::FULL:
      ans.presence = InsertResult::Presence::PRESENT;
      return ans;
    default:
      assert(false);
    }
  }

  bool displace(const Key& k) {
    Slot& s = data[locate(k)];
    if (Status::FULL == s.status) {
      ++tombstoned_count;
      --full_count;
      s.status = Status::DELETED;
      return true;
    }
    return false;
  }


  void upsize() {
    const auto new_size = (0 == capacity) ? 1 : (capacity * up_scale);
    resize(new_size);
  }

  void downsize() {
    const auto new_size = (1 == capacity) ? 0 : (capacity / down_scale);
    resize(new_size);
  }

  void samesize() {
    resize(capacity);
  }

  void resize(const size_t new_size) {
    LinearHashSet* that = new LinearHashSet(new_size);
    for (size_t i = 0; i < capacity; ++i) {
      if (Status::FULL == data[i].status) {
        const InsertResult res = that->place(data[i].key);
        assert (InsertResult::Presence::ABSENT == res.presence);
      }
    }
    swap(that);
    delete that;
  }

  // TODO: overload for std::swap
  // TODO: copy and move and assignment constructors
  void swap(LinearHashSet * that) {
    std::swap(data, that->data);
    std::swap(capacity, that->capacity);
    std::swap(full_count, that->full_count);
    std::swap(tombstoned_count, that->tombstoned_count);
  }


public:
  ~LinearHashSet() {
    if (NULL != data) {
      free(data);
      data = NULL;
    }
  }

  void erase(const iterator s) {
    s->status = Status::TOMBSTONED;
    s->key.~Key(); // TODO: make sure the right destructors are called, the right move constructors are called
    if (full_count * down_limit < capacity) {
      downsize();
    } else if (tombstoned_count * tombstoned_limit > capacity) {
      samesize();
    }    
  }

  InsertResult cover(const Key& k) {
    if (full_count == capacity) {
      assert (capacity <= 1);
      upsize();
    } else if (full_count * up_limit > capacity) {
      upsize();
    }

    return place(k);
  }

  void insert(iterator const s) {
  }

  iterator find(const Key& k) {
    if (0 == capacity) return nullptr;
    const size_t i = locate(k);
    if (Status::FULL == data[i].status) {
      return &data[i];
    }
    return nullptr;
  }


};

template<typename Key, typename Hash, typename Equal>
void swap(LinearHashSet<Key, Hash, Equal>& x, LinearHashSet<Key,Hash,Equal>& y) {
  x.swap(y);
}


#endif // LINEAR_HASH_SET
