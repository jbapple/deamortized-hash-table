#ifndef DEAMORTIZED_MAP
#define DEAMORTIZED_MAP

#include <cstddef>
#include <utility>
#include <cstdlib>
#include <memory>

#include "node.hh"
#include "tlsf_allocator.hpp"

template<typename Key, typename Val, typename Allocator, typename Less>
struct deamortized_map {

  typedef Node<Key, Val, Allocator, Less> TreeNode;

  TreeNode *root, *head;

  deamortized_map() : root(TreeNode::bottom()), head(NULL) {}

  TreeNode* find(const Key& key) const { 
    return root->find(key);
  }

  std::pair<bool,TreeNode*> soft_insert(const Key& key, const Val& val) {
    const auto ia = root->soft_insert(key, val);
    if (ia.is_new) {
      root = ia.new_root;
      if (not head or not ia.inserted->pred) {
        head = ia.inserted;
      }
    }
    return std::make_pair(ia.is_new, ia.inserted);
  }

  void dealloc_one() {
    root = NULL;
    if (head) {
      TreeNode* tmp = head->succ;
      TreeNode::allocator.destroy(head);
      TreeNode::allocator.deallocate(head, 1);
      head = tmp;
    }
  }
  
  ~deamortized_map() {
    while (head) dealloc_one();
  }

};

#endif
