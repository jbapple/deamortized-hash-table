#include <cstdint>

//size_t hash(int x) {return static_cast<size_t>(x);}

template<typename Key, size_t up_limit = 2, size_t up_scale = 2, size_t down_limit = 8, size_t down_scale = 2, size_t deleted_limit = 3>
struct hash_map {
  enum Status {
    DELETED, EMPTY, FULL
  };
  struct slot {
    Key key;
    Status status;
  };
  slot * data;
  size_t capacity, full, deleted;
  hash_map()
    : data(NULL), capacity(0), full(0), deleted(0) {}
  hash_map(const size_t capacity)
    // use malloc to allow Key type without default constructor
    : data(reinterpret_cast<slot*>(malloc(sizeof(slot) * capacity))), 
      capacity(capacity), full(0), deleted(0) 
  {
    assert (NULL != data);
    for (size_t i = 0; i < capacity; ++i) {
      data[i].status = EMPTY;
    }
  }
  
private: 

  // find a key in a nonempty table
  size_t locate(const Key& k) {
    assert (nullptr != data); assert (capacity > 0); assert (0 == (capacity & capacity-1));
    const auto cap_mask = capacity-1;
    auto h = hashf(k) & (cap_mask);
    while (true) {
      if (data[h].status != FULL) return h;
      if (data[h].key == k) return h;
      h = (h+1) & cap_mask;
    }
  }

  // place a key
  void place(const Key& k) {
    assert (full + deleted < capacity);
    const auto i = locate(k);
    switch (data[i].status) {
    case DELETED:
      --deleted;
      // fall through:
    case EMPTY:
      data[i].status = FULL;
      data[i].key = k;
      ++full;
      break;
    case FULL:
    default:
      break;
    }
  }

  void displace(const Key& k) {
    const auto i = locate(k);
    if (FULL == data[i].status) {
      ++deleted;
      --full;
      data[i].Status = DELETED;
    }
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
    hash_map* that = new hash_map(new_size);
    for (size_t i = 0; i < capacity; ++i) {
      if (FULL == data[i].status) {
        that->place(data[i].key);
      }
    }
    swap(that);
    delete that;
  }
  
  void swap(hash_map * that) {
    std::swap(data, that->data);
    std::swap(capacity, that->capacity);
    std::swap(full, that->full);
    std::swap(deleted, that->deleted);
  }


public:
  ~hash_map() {
    if (NULL != data) {
      free(data);
      data = NULL;
    }
  }

public: 

  void insert(const Key& k) {
    if (full == capacity) {
      upsize();
    }
    place(k);
    if (full * up_limit > capacity) {
      upsize();
    }
  }

  void remove(const Key& k) {
    displace(k);
    if (full * down_limit < capacity) {
      downsize();
    } else if (deleted * deleted_limit > capacity) {
      samesize();
    }
  }

  Key* find(const Key& k) {
    if (NULL == data) {
      return nullptr;
    }
    const auto i = locate(k);
    if (FULL == data[i].status) {
      return &data[i].key;
    }
    return nullptr;
  }

};
