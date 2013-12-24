template<typename Key, typename Val>
struct deamortized_map {
  deamortized_map();
  struct Cell {
    Key k;
    Value v;
  };
  KillNode {
    Node * node;
    KillNode * next;
  };
  KillNode kill_list;
  Node {
    Cell data;
    Node *left, *right;
    size_t height;
  };
  Cell * find(const Key&) const;
  Cell * insert(const Key&, const Val&);
  size_t size;

  // simpler idea: track the order the keys came in!

  Cursor {
    Node * node;
    Cursor * up;
    // DFS in O(1) worst-case time
    bool advance() {
      if (not node) return false;
      if (not node->left) {
	if (not node->right) {
	  if (not up) return false;
	  node = up->node->right;
	  up = up->up;
	  return advance();
	}
	node = node->right;
	
	  
    }
  };


  void kill_one() {
    Node *left_orphan = kill_list.node->left, 
      *right_orphan = kill_list.node->right;
    delete kill_list->node;
    if (right_orphan and not left_orphan) { std::swap(left_orphan, right_orphan); }
    if (right_orphan) {
      KillNode * head = new KillNode();
      head->node = right_orphan;
      head->next = kill_list->next;
      kill_list->next = head;
      kill_list->node = left_orphan;
    } else if (left_orphan) {
      kill_list->node = left_orphan;
    } else if (kill_list->next) {
      kill_list->node = kill_list->next->node;
      kill_list->next = kill_list->next->next;
      delete kill_list->next;
    }
    --size;
  }
};
