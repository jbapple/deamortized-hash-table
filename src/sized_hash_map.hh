#include <cstdlib>

template<typename Key, typename Val>
struct sized_hash_map {
  sized_hash_map(size_t slot_count) : 
    slot_count(slot_count), 
    cell_count(0),
    tombstone_count(0),
    initialized(0),
    head(NULL),
    slots(std::malloc(slot_count * sizeof(Slot *)))
  {}

  void resize(size_t new_slot_count) {
    std::free(slots);
    new (this) sized_hash_map(new_slot_count);
  }

  ~sized_hash_map() {
    deinitialize(1);
    std::free(slots);
  }

  initialize(size_t time_left) {
    const size_t todo = (slot_count + (time_left - 1))/time_left;
    for (size_t i = 0; i < todo and initialized < slot_count; ++i) {
      new (slots + initialized) Slot();
      ++initialized;
    }
  }
  deinitialize(size_t time_left) {
    const size_t todo = (slot_count + cell_count + (time_left - 1))/time_left;
    for (size_t i = 0; i < todo and initialized >= 0; ++i) {
      if (slots[initialized].bucket.head) {
	Cell * const new_head = slots[initialized].bucket.head->next;
	delete slots[initialized].bucket.head;
	slots[initialized].bucket.head = new_head;
      } else {
	//slots[initialized].bucket.~Bucket();
	--initialized;
      }
    }
  }

  size_t slot_count;
  size_t cell_count;
  size_t tombstone_count;
  signed long initialized;
  Slot *head;
  typedef deamortized_map<Key, std::shared_ptr<Val> > Bucket;
  struct Slot {
    Bucket bucket;
    Slot *prev, *next;
  };
  typedef typename Bucket::Cell Cell;
  Slot * slots;

  Cell * find(const Key & k) const {
    return slots[hash(k) & (slot_count - 1)].find(k);
  }

  void weak_delete(Cell * c) {
    if (not c->val) return;
    ++tombstone_count;
    c->val.reset();
    c->prev = c->prev ? c->prev->prev : NULL;
    c->next = c->next ? c->next->next : NULL;
    if (c->prev == c->next) {
      slots[hash(c->key)].prev = slots[hash(c->key)].prev ? slots[hash(c->key)].prev->prev : NULL;
      slots[hash(c->key)].next = slots[hash(c->key)].next ? slots[hash(c->key)].next->next : NULL;
    }
  }

  std::pair<bool, Cell *> weak_insert(const Key& k, const Val& v) {
    const auto slot = slots[hash(k) & (slot_count - 1)];
    const auto ans = slot.bucket.insert(k, std::make_shared<Val>(v));
    if (ans.first) {
      ++cell_count;
      
      return there;
    } else {
      return slots[hash(k) & (capacity-1)].insert(k, shared_ptr<Val>(new Val(v)));
    }
  }
};

