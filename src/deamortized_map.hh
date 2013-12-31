#include <cstddef>
#include <utility>
#include <cstdlib>
#include <memory>

#include "node.hh"

template<typename Key, typename Val>
struct deamortized_map {

  struct NodeTracker {
    Node<Key,Val,NodeTracker> *prev, *next;
    NodeTracker(Node<Key,Val,NodeTracker> *prev = NULL, Node<Key,Val,NodeTracker> *next = NULL) :
      prev(prev), next(next) {}
  };

  typedef Node<Key,Val,NodeTracker> TreeNode;
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

};
