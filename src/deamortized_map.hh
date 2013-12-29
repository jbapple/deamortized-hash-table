#include <cstddef>
#include <utility>
#include <cstdlib>

template<typename Key, typename Val>
struct deamortized_map {
  struct Node {
    Key key;
    Val val;
    Node *left, *right; // tree structure
    Node *prev, *next; // for iterating
    std::size_t level; // for balance
    Node(const Key& key, const Val& val) 
      : key(key), val(val),
	left(bottom()), right(bottom()), prev(NULL), next(NULL), level(1) {}
  };

  // AA trees:
  static Node * skew(Node * root) {
    if (root->left->level == root->level) {
      // rotate right
      Node * ans = root->left;
      root->left = ans->right;
      ans->right = root;
      return ans;
    }
    return root;
  }

  static Node * split(Node * root) {
    if (root->right->right->level == root->level) {
      //rotate left
      Node * ans = root->right;
      root->right = ans->left;
      ans->left = root;
      ans->level += 1;
      return ans;
    }
    return root;
  }

  struct InsertAns {
    bool is_new;
    Node * location;
    Node * new_root;
  };

  static InsertAns insert(const Key& key, const Val& val, Node * root) {
    if (0 == root->level) {
      InsertAns ans;
      // link up node later
      ans.location = new Node(key, val);
      ans.new_root = ans.location;
      ans.is_new = true;
      return ans;
    }
    InsertAns ans;
    if (key < root->key) {
      ans = insert(key, val, root->left);
      root->left = ans.new_root;
    } else if (root->key < key) {
      ans = insert(key, val, root->right);
      root->right = ans.new_root;
    } else {
      ans.is_new = false;
      ans.location = root;
    }
    ans.new_root = root;
    if (ans.is_new) {
      ans.new_root = skew(ans.new_root);
      ans.new_root = split(ans.new_root);
    }
    return ans;
  }

  static Node * find(const Key& key, Node* const root) {
    if (0 == root->level) return NULL;
    if (key < root->key) return find(key, root->left);
    if (root->key < key) return find(key, root->right);
    return root;
  }

  Node *head, *root;
  std::size_t size;
  deamortized_map() : head(NULL), root(bottom()), size(0) {}
  Node * find(const Key& key) const {
    return find(key, root);
  }
  std::pair<bool,Node*> insert(const Key& key, const Val& val) {
    InsertAns ia = insert(key, val, root);
    if (ia.is_new) {
      ia.location->next = head;
      ia.location->prev = NULL;
      if (head) head->prev = ia.location;
      head = ia.location;
      root = ia.new_root;
      ++size;
    }
    return std::make_pair(ia.is_new, ia.location);
  }

  static Node * bottom() {
    static Node * ans = NULL;
    if (NULL == ans) {
      ans = reinterpret_cast<Node *>(std::malloc(sizeof(Node)));
      ans->left = ans->right = ans;
      ans->prev = ans->next = NULL;
      ans->level = 0;
    }
    return ans;
  }
};
