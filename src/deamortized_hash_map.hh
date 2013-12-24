#include <memory>

template<typename Key, typename Val>
struct base_hash_map {
  base_hash_map();
  struct Unit;
  Unit* find(const Key &);
  void weak_erase(Unit* location);
  Unit* insert(const Key&, const Val &);
};

template<typename Key, typename Val>
struct sized_hash_map {
  sized_hash_map(size_t capacity) : 
    capacity(capacity), 
    data(malloc(capacity * sizeof(Bucket *))),
    initialized(0), size(0) ;
  {}

  void resize(size_t capacity) {
    free(data);
    data = malloc(capacity * sizeof(Bucket *));
  }

  initialize(size_t time_left) {
    const size_t todo = (capacity + (time_left - 1))/time_left;
    for (size_t i = 0; i < todo and initialized < capacity; ++i) {
      new (data + initialized) Bucket();
      ++initialized;
    }
  }
  deinitialize(size_t time_left) {
    const size_t todo = (capacity + size + (time_left - 1))/time_left;
    for (size_t i = 0; i < todo and initialized >= 0; ++i) {
      if (data[initialized].size > 0) {
	data[initialized].kill_one();
      } else {
	data[initialized].~Bucket();
	--initialized;
      }
    }
  }

  size_t capacity;
  signed long initialized;
  typedef deamortized_map<Key, std::shared_ptr<Val> > Bucket;
  typedef typename Bucket::Cell Cell;
  Bucket * data;

  Cell * find(const Key & k) const {
    return data[hash(k) & (capacity - 1)].find(k);
  }
  // weak_delete is just clear out the value in the shared_pointer
  Cell * weak_insert(const Key& k, const Val& v) {
    Cell * const there = data[hash(k) & (capacity-1)].find(k);
    if (there) {
      there->val = shared_ptr<Val>(new Val(v));
      return there;
    } else {
      return data[hash(k) & (capacity-1)].insert(k, shared_ptr<Val>(new Val(v)));
    }
  }
};

