#include <cstddef>
#include <utility>
#include <cstdlib>
#include <memory>

#include "node.hh"

template<typename Key, typename Val>
struct deamortized_map {

  struct NodeTracker {
    Val val;
    Node<Key,NodeTracker> *prev, *next;
    NodeTracker(const Val& val, Node<Key,NodeTracker> *prev, Node<Key,NodeTracker> *next) :
      val(val), prev(prev), next(next) {}
  };

  Node<Key,NodeTracker> *head, *root;
 
  deamortized_map() : head(NULL), root(Node<Key,NodeTracker>::bottom()) {}

  Node<Key,NodeTracker>* find(const Key& key) const { 
    return root->find(key);
  }

  std::pair<bool,Node<Key,NodeTracker>*> insert(const Key& key, const Val& val) {
    const auto ia = root->insert(key, NodeTracker(val, NULL, head));
    if (ia.is_new) {
      if (ia.inserted->next) ia.inserted->next->prev = ia.inserted;
      head = ia.inserted;
    }
    return make_pair(ia.is_new, ia.inserted);
  }

};
