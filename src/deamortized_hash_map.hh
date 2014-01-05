#include <memory>
#include <cassert>

#include "sized_hash_map.hh"

template<typename Key, typename Val>
struct base_hash_map {
  enum {
    DEINIT, INIT, REBUILD
  } state;

  size_t state_counter;

  sized_hash_map<Key, Val> *here, *there, alpha, bravo;
  typedef typename sized_hash_map<Key, Val>::DNode Cell;

  Cell * rebuild_tracker;

  base_hash_map();

  static size_t slot_count_for_live_set(size_t n) {
    size_t i = 0;
    // TODO: improve this timing
    while (n >> i) ++i;
    return ((size_t)1) << i;
  }

  void step() {
    switch (state) {
    case DEINIT: {
      if (0 == state_counter) {
        while (there->initialized > 0) {
          there->deinitialize_one();
        }
        state = INIT;
        state_counter = (here->node_count - here->tombstone_count)/8;
        there->resize(slot_count_for_live_set(here->node_count - here->tombstone_count));
      } else {
        const size_t work_left = there->node_count + there->slot_count;
        const size_t do_now = (work_left + state_counter - 1)/state_counter;
        for (size_t i = 0; i < do_now; ++i) {
          there->deinitialize_one();
        }
        --state_counter;
      }
    }
    case INIT: {
      if (0 == state_counter) {
        while (there->initialized < there->slot_count) {
          there->initialize_one();
        }
        state = REBUILD;
        state_counter = (here->node_count - here->tombstone_count)/8;
        rebuild_tracker = here->live_head;
      } else {
        const size_t work_left = there->node_count;
        const size_t do_now = (work_left + state_counter - 1)/state_counter;
        for (size_t i = 0; i < do_now; ++i) {
          there->initialize_one();
        }
        --state_counter;
      }
    }
    case REBUILD: {
      if (0 == state_counter) {
        while (rebuild_tracker) {
          there->insert(rebuild_tracker->key, rebuild_tracker->val);
          rebuild_tracker = rebuild_tracker->live_next;
        }
        state_counter = (there->node_count - there->tombstone_count)/4;
        state = DEINIT;
        std::swap(here, there);
      } else {
        const size_t work_left = here->node_count - here->tombstone_count;
        const size_t do_now = (work_left + state_counter - 1)/state_counter;
        for (size_t i = 0; i < do_now; ++i) {
          if (rebuild_tracker) {
            there->insert(rebuild_tracker->key, rebuild_tracker->val);
            rebuild_tracker = rebuild_tracker->live_next;
          }
        }
        --state_counter;
      }
    }
    }
  }

  Cell * find(const Key & k) {
    return here->find(k);
  }
    
  bool erase(Cell* location) {
    bool ans = false;
    if (location) {
      if (location == rebuild_tracker) {
        rebuild_tracker = rebuild_tracker->live_next;
      }
      ans = here->erase(location);
      if (ans and REBUILD == state) {
        Cell * where = there->find(location->key);
        if (where) there->erase(where);
      }
    }
    step();
    return ans;
  }

  std::pair<bool,Cell*> insert(const Key& k, const Val & v) {
    auto val = std::make_shared(v);
    auto ans = here->insert(k, val);
    if (ans.first and REBUILD == state) {
      there->insert(k, val);
    }
    step();
    return ans;
  }
};

