#include <cstdlib>

// AA trees:
template<typename Key, typename Val>
struct Node {
  Key key;
  Val val;
  Node *left, *right;
  std::size_t level; // for balance
  Node(const Key& key, const Val& val) 
      : key(key), val(val),
	left(bottom()), right(bottom()), 
	level(1) {}

  static Node * bottom() {
    static Node * ans = NULL;
    if (NULL == ans) {
      ans = reinterpret_cast<Node *>(std::malloc(sizeof(Node)));
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

  InsertAns insert(const Key& k, const Val& v) {
    InsertAns ans;
    if (0 == level) {
      ans.is_new = true;
      ans.inserted = new Node(k, v);
      ans.new_root = ans.inserted;
      return ans;
    }
    if (k < key) {
      ans = left->insert(k, v);
      left = ans.new_root;
    } else if (key < k) {
      ans = right->insert(k, v);
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
    if (k < key) return left->find(k);
    if (key < k) return right->find(k);
    return this;
  }
};
