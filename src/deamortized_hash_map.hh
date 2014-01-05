#include <memory>
#include <cassert>
#include <iostream>
#include <utility>

#include "sized_hash_map.hh"

// TODO: destructor

template<typename Key, typename Val, typename Hasher = std::hash<Key>, typename Allocator = TlsfAllocator<char>, typename Less = std::less<Key> >
struct base_hash_map {
  enum {
    DEINIT, INIT, REBUILD
  } state;

  size_t time_left_in_state;
  size_t work_left_in_state;

  sized_hash_map<Key, Val, Hasher, Allocator, Less> *here, *there, alpha, bravo;
  typedef typename sized_hash_map<Key, Val, Hasher, Allocator, Less>::DNode Cell;

  Cell * rebuild_tracker;

  base_hash_map() : state(DEINIT), time_left_in_state(16), work_left_in_state(32), here(&alpha), there(&bravo), alpha(32), bravo(32), rebuild_tracker(NULL) {}


  static size_t slot_count_for_live_set(size_t n) {
    size_t i = 0;
    // TODO: improve this timing
    while (n >> i) ++i;
    //std::cout << "resize: " << n << " to " << (((size_t)1) << i) << std::endl;
    return ((size_t)1) << i;
  }

  void step() {
    switch (state) {
    case DEINIT: {
      assert (here->tombstone_count <= here->node_count/2 + 1);
      assert (here->slot_count*2 + 1>= here->node_count);
      time_left_in_state = std::min((here->node_count/2 + 1 - here->tombstone_count)/4, (1 + here->slot_count * 2 - here->node_count)/2);
      //std::cout << "time_left_in_state: " << time_left_in_state << ", " 
      //          << here->slot_count << ' ' << here->node_count << ' ' << here->tombstone_count << std::endl;
      if (0 == time_left_in_state) {
        while (there->initialized > 0) {
          there->deinitialize_one();
        }
        state = INIT;
        there->resize(slot_count_for_live_set(here->node_count - here->tombstone_count));
        time_left_in_state = (here->node_count - here->tombstone_count)/8;
        work_left_in_state = there->slot_count;
      } else {
        const size_t do_now = (work_left_in_state + time_left_in_state - 1)/time_left_in_state;
        for (size_t i = 0; i < do_now; ++i) {
          there->deinitialize_one();
          --work_left_in_state;
        }
        --time_left_in_state;
      }
      break;
    }
    case INIT: {
      if (0 == time_left_in_state) {
        while (there->initialized < there->slot_count) {
          there->initialize_one();
        }
        state = REBUILD;
        time_left_in_state = (here->node_count - here->tombstone_count)/8;
        work_left_in_state = here->node_count - here->tombstone_count;
        rebuild_tracker = here->live_head;
      } else {
        const size_t do_now = (work_left_in_state + time_left_in_state - 1)/time_left_in_state;
        for (size_t i = 0; i < do_now; ++i) {
          there->initialize_one();
          --work_left_in_state;
        }
        --time_left_in_state;
      }
      break;
    }
    case REBUILD: {
      if (0 == time_left_in_state) {
        while (rebuild_tracker) {
          there->insert(rebuild_tracker->key, rebuild_tracker->val);
          rebuild_tracker = rebuild_tracker->live_next;
        }
        work_left_in_state = here->node_count + here->slot_count;
        assert (there->tombstone_count <= there->node_count/2);
        assert (there->slot_count*2 >= there->node_count);
        time_left_in_state = std::min(there->node_count/2 - there->tombstone_count, 
                                      there->slot_count * 2 - there->node_count);
        state = DEINIT;
        std::swap(here, there);
      } else {
        const size_t do_now = (work_left_in_state + time_left_in_state - 1)/time_left_in_state;
        for (size_t i = 0; i < do_now; ++i) {
          if (rebuild_tracker) {
            there->insert(rebuild_tracker->key, rebuild_tracker->val);
            rebuild_tracker = rebuild_tracker->live_next;
            --work_left_in_state;
          }
        }
        --time_left_in_state;
      }
      break;
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
        there->erase(there->find(location->key));
      }
    }
    if (ans) step();
    return ans;
  }

  std::pair<bool,Cell*> insert(const Key& k, const Val & v) {
    auto val = std::make_shared<Val>(v);
    auto ans = here->insert(k, val);
    if (ans.first and REBUILD == state) {
      there->insert(k, val);
    }
    if (ans.first) step();
    return ans;
  }
};

