#ifndef DEAMORTIZED_HASH_MAP_HH
#define DEAMORTIZED_HASH_MAP_HH

#include <memory>
#include <cassert>
#include <iostream>
#include <utility>

#include "sized_hash_map.hh"

template<typename Key, typename Val, typename Hasher = std::hash<Key>, typename Allocator = TlsfAllocator<char>, typename Less = std::less<Key> >
struct base_hash_map {
  enum {
    DEINIT, INIT, REBUILD
  } state;

  size_t time_left_in_state;
  size_t work_left_in_state;

  struct LiveCell;
  typedef typename sized_hash_map<Key, LiveCell, Hasher, Allocator, Less> SubMap;
  SubMap here, there;
  typedef typename SubMap::DNode Cell;
  struct LiveCell {
    LiveCell *next, *prev;
    Cell *here, *there;
    Val val;
    LiveCell(vonst Val& val) : val(val), next(NULL), prev(NULL), here(NULL), there(NULL) {}
  };
  LiveCell *head, *tail, *cursor;
  size_t here_tombstones, there_tombstones;

  typename Allocator::template rebind<LiveCell>::other lcalloc;

  base_hash_map() : 
    state(DEINIT), 
    time_left_in_state(1), work_left_in_state(1), 
    here(2), there(2),
    head(NULL), tail(NULL), cursor(NULL),
    here_tombstones(0), there_tombstones(0)
  {}

  void destroy_cell(LiveCell* const c) {
    if (c->next) c->next->prev = c->prev;
    if (c->prev) c->prev->next = c->next;
    if (c == head) head = c->next;
    if (c == tail) tail = c->prev;
    //n->next = c->prev = NULL;
    if (c->here) {
      c->here->val = NULL;
      ++here_tombstones;
    }
    if (c->there) {
      c->there->val = NULL;
      ++there_tombstones;
    }
    //c->backrefs[0] = c->backrefs[1] = NULL;
    lcalloc.destroy(c);
    lcalloc.deallocate(1, c);
  }

  ~base_hash_map() {
    while (head) {
      auto new_head = head->next;
      destroy_cell(head);
      head = new_head;
    }
  }

  Cell * find(const Key & k) {
    return here.find(k);
  }

  bool erase(Cell* location) {
    if (not location) return false;
    if (not location->val) return false;
    if (location->val->here != location) {
      swap(location->val->here, location->val->there);
    }
    destroy_cell(location->val);
    step();
    return true;
  }

  std::pair<bool,Cell*> insert(const Key& k, const Val & v) {
    auto ans = here->insert(k, NULL);
    if (not ans.first and not ans.second.val) {
      ans.first = true;
      --here_tombstones;
    }
    if (ans.first) {
      ans.second->val = lcalloc.allocate(1);
      lcalloc.construct(ans.second->val, v);
      if (REBUILD == state) {
        there->insert(k, ans.second->val);
      }
      step();
    }
    return ans;
  }

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


};

//template<typename Key, typename Val, typename Hasher, typename Allocator, typename Less>
//typename Allocator::template rebind<Val>::other base_hash_map<Key, Val, Hasher, Allocator, Less>::vallocator;

#endif
