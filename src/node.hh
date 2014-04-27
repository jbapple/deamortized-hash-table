#ifndef NODE_HH
#define NODE_HH

#include <cstdlib>

/*

I would like to maintain a set with worst-case constant time deletion at a known location and logarithmic-time insert and find.
This can be done using a construction of Overmars by building upon a structure with logarithmic-time insert and find and constant-time "weak delete".
"Weak delete" is an operation in which a key is deleted without increasing the running times of the costs of the other operations.

Let m be the number of inserts that have occured on the structure and, as usual, let n be the number of keys resident in the structure.
It may be the case that n is much less than m.
Insert and find take O(\lg m) time in the worst case, but not necessarily O(\lg n) time.

A structure like this is the building block of the Overmars construction.

What I have described so far can be implemented with common binay search trees like red-black trees and AVL trees.
The idea is to associate an extra bit of data with each node, the life bit.
If the life bit is 0, then the node is just a placeholder - it does not rpresent a key that is present in the set.
Searches that terminate at that node should return false.
If the life bit is 1, then the node represents a key in the set, just as usual.

template <typename Key>
struct WeakDeleteTree {
  std::map<Key, bool> data;
  typename std::map<Key, bool>::iterator iterator;

  std::pair<bool, iterator> insert(const Key& key) {
    return data.insert(key, true);
  }

  bool weak_delete(const iterator& i) {
    const bool ans = i->second;
    i->second = false;
    return ans;
  }

  iterator find(const Key& key) {
    const iterator i = data.find(key);
    if (data.end() == i or not i->second) return data.end();
    return i;
  }
};

For a language like C++, this does not match the semantics of the standard container library's set.
The difference is the lifetime of the keys.
In a standard container, once a key has been deleted, it's destructor is invoked and the memory it occupied can be reclaimed by the allocator.
In WeakDeleteTree, keys live on indefinitely.

*/

// AA trees:
template<typename Key, typename Val, typename Allocator, typename Less>
struct Node {
  static const Less lesser;
  static typename Allocator::template rebind<Node<Key,Val,Allocator,Less> >::other allocator;
  Key * key;
  Val val;
  enum {
    ALIVE, DEAD_LEADER, DEAD_EDGE, DEAD_MIDDLE
  } condition;
  Node *left, *right;
  Node *pred, *succ;
  Node *life_left, *life_right;
  std::size_t level; // for balance
  Node(const Key& key, const Val& val) 
    : key(key), val(val),
      condition(ALIVE),
      left(bottom()), right(bottom()),
      pred(NULL), succ(NULL),
      life_left(NULL), life_right(NULL),
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

  InsertAns soft_insert(const Key& k, const Val& v, 
                        Node * const before = NULL, Node * const after = NULL,
                        Node * const live_before = NULL, Node * const live_after = NULL) {
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
      ans.inserted->life_left = live_before;
      ans.inserted->life_right = live_after;
      if (live_before) live_before->life_right = ans.inserted;
      if (live_after) live_after->life_left = ans.inserted;
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
