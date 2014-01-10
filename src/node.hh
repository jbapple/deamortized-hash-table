#ifndef NODE_HH
#define NODE_HH

#include <cstdlib>

// AA trees:
template<typename Key, typename Val, typename Allocator, typename Less>
struct Node {
  static const Less lesser;
  static typename Allocator::template rebind<Node<Key,Val,Allocator,Less> >::other allocator;
  const Key key;
  Val val;
  Node *left, *right;
  Node *pred, *succ;
  std::size_t level; // for balance
  Node(const Key& key, const Val& val) 
    : key(key), val(val),
      left(bottom()), right(bottom()),
      pred(NULL), succ(NULL),
      level(1) {}

  static Node * bottom() {
    // TODO: this is a memory leak, kindof
    static Node * ans = NULL;
    if (NULL == ans) {
      ans = allocator.allocate(1);
      ans->left = ans->right = ans;
      ans->level = 0;
    }
    return ans;
  }

  Node * skew() {
    if (left->level == level) {
      // rotate right
      Node * ans = left;
      left = ans->right;
      ans->right = this;
      return ans;
    }
    return this;
  }

  Node * split() {
    if (right->right->level == level) {
      //rotate left
      Node * ans = right;
      right = ans->left;
      ans->left = this;
      ans->level += 1;
      return ans;
    }
    return this;
  }

  struct InsertAns {
    bool is_new;
    Node * inserted;
    Node * new_root;
  };

  InsertAns soft_insert(const Key& k, const Val& v, Node * const before = NULL, Node * const after = NULL) {
    InsertAns ans;
    if (0 == level) {
      ans.is_new = true;
      ans.inserted = allocator.allocate(1);
      allocator.construct(ans.inserted, Node(k, v));
      ans.new_root = ans.inserted;
      ans.inserted->pred = before;
      ans.inserted->succ = after;
      if (before) before->succ = ans.inserted;
      if (after) after->pred = ans.inserted;
      return ans;
    }
    if (lesser(k, key)) {
      ans = left->soft_insert(k, v, before, this);
      left = ans.new_root;
    } else if (lesser(key, k)) {
      ans = right->soft_insert(k, v, this, after);
      right = ans.new_root;
    } else {
      ans.is_new = false;
      ans.inserted = this;
    }
    ans.new_root = this;
    if (ans.is_new) {
      ans.new_root = ans.new_root->skew();
      ans.new_root = ans.new_root->split();
    }
    return ans;
  }

  Node * find(const Key& k) {
    if (0 == level) return NULL;
    if (lesser(k, key)) return left->find(k);
    if (lesser(key, k)) return right->find(k);
    return this;
  }
};

template<typename Key, typename Val, typename Allocator, typename Less>
typename Allocator::template rebind<Node<Key,Val,Allocator,Less> >::other Node<Key,Val,Allocator,Less>::allocator = typename Allocator::template rebind<Node<Key,Val,Allocator,Less> >::other();

template<typename Key, typename Val, typename Allocator, typename Less>
const Less Node<Key,Val,Allocator,Less>::lesser = Less();

#endif
