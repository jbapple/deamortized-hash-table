#include <cstddef>
#include <utility>
#include <cstdlib>
#include <memory>

#include "node.hh"
#include "tlsf_allocator.hpp"

template<typename Key, typename Val, typename Allocator = TlsfAllocator<char> >
struct deamortized_map {

  struct NodeTracker;

  typedef Node<Key,Val,NodeTracker,Allocator> TreeNode;

  struct NodeTracker {
    TreeNode *prev, *next;
    NodeTracker(TreeNode *prev = NULL, TreeNode *next = NULL) :
      prev(prev), next(next) {}
  };

  TreeNode *head, *root;
 
  deamortized_map() : head(NULL), root(TreeNode::bottom()) {}

  TreeNode* find(const Key& key) const { 
    return root->find(key);
  }

  std::pair<bool,TreeNode*> insert(const Key& key, const Val& val) {
    const auto ia = root->insert(key, val);
    if (ia.is_new) {
      ia.inserted->next = head;
      if (ia.inserted->next) ia.inserted->next->prev = ia.inserted;
      head = ia.inserted;
      root = ia.new_root;
    }
    return std::make_pair(ia.is_new, ia.inserted);
  }

  void dealloc_one() {
    if (head) {
      TreeNode* tmp = head->next;
      TreeNode::allocator.destroy(head);
      TreeNode::allocator.deallocate(head, 1);
      head = tmp;
    }
  }
  
  ~deamortized_map() {
    while (head) dealloc_one();
  }

};
