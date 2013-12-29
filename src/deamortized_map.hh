template<typename Key, typename Val>
struct deamortized_map {
  deamortized_map();
  struct Cell {
    Key key;
    Value val;
  };
  Node {
    Cell data;
    Node *left, *right, *next;
    size_t height;
  };

  Node * head;
  Cell * find(const Key&) const;
  Cell * insert(const Key&, const Val&);
  size_t size;
};

template<typename Key, typename Val>
struct insert_ans {
  bool success;
  typename deamortized_map<Key,Val>::Node *here, *ans;
}

template<typename Key, typename Val>
insert_ans<Key,Val> node_insert(typename deamortized_map<Key,Val>::Node * root, const Key& key, const Val& val) {
  if (root->right && root->right->right && root->right->right->height == root->height) {
    typename deamortized_map<Key,Val>::Node * new_root = root->right;
    root->right = new_root->left;
    new_root->left = root;
    new_root->height += 1;
    return node_insert(new_root, key, val);
  }
  if (root) {
    if (key == root->key) return {false, root, root};
    if (key < root->key) return node_insert
  }
}

// TODO: sentinel

template<typename Key, typename Val>
std::pair<bool,Node *> node_insert(typename deamortized_map<Key,Val>::Node * root, const Key& key, const Val& val) {
  if (not root) {
    auto * ans = new typename deamortized_map<Key,Val>::Node();
    ans->data->key = key;
    ans->left = ans->right = ans->next = nullptr;
    ans->height = 0;
    return std::make_pair(true, ans);
  }
  if (key == root->data->key) {
    return std::make_pair(false, root);
  }
  if (key < root->data->key) {
    if (left_heavy(root)) { root = rotate_right(root); }
    root->left = node_insert(root->left, key, val).second;
    reheight(root);
    return root;
  }
    
}

template<typename Key, typename Val>
typename deamortized_map<Key,Val>::Node * rotate_right(typename deamortized_map<Key,Val>::Node * root) {
  // single rotation:
  if ((not root->left->left) or (not root->left->right) or (root->left->left->height >= root->left->right->height)) {
    typename deamortized_map<Key,Val>::Node * const ans = root->left;
    root->left = ans->right;
    ans->right = root;
    reheight(root);
    reheight(ans);
    return ans;
  } else { // double rotation
    typename deamortized_map<Key,Val>::Node * const ans = root->left->right;
    root->left->right = ans->left;
    ans->left = root->left;
    root->left = ans->right;
    ans->right = root;
    reheight(ans->left);
    reheight(ans->right);
    reheight(ans);
    return ans;
  }
}
