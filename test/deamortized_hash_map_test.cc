#include <map>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <limits>

#include <time.h>

#include "../src/deamortized_hash_map.hh"

void copy_map_find_test() {
  base_hash_map<char, int> actual;
  std::map<char, int> expected;
  for (size_t i = 0; i < 100; ++i) {
    const char key = std::rand();
    const int val = std::rand();
    //std::cout << "key: " << static_cast<int>(key) << std::endl;
    actual.insert(key, val);
    auto j = expected.find(key);
    if (j == expected.end()) expected.insert(std::make_pair(key, val));
    //print_map_keys(actual);
    const char tester = std::rand();
    /*
    std::cout << "tester: " << static_cast<int>(tester)
	      << std::boolalpha
	      << '\t' << (actual.find(tester) == NULL)
	      << '\t' << (expected.find(tester) == expected.end())
	      << std::endl;
    */
    assert ((actual.find(tester) == NULL) == (expected.find(tester) == expected.end()));
  }
}

/*
template<typename Key, typename Val>
size_t iterator_length(const dmap<Key,Val>& m) {
  size_t ans = 0;
  auto* here = m.head;
  while (here) {
    ++ans;
    if (here->next) assert (here->next->prev == here);
    here = here->next;
  }
  return ans;
}


void iterator_test() {
  dmap<char, bool> actual;
  size_t size = 0;
  for (size_t i = 0; i < 100; ++i) {
    const char key = std::rand();
    if (actual.insert(key, true).first) ++size;
    assert (iterator_length(actual) == size);
  }  
}

template<typename Node>
size_t node_depth(const Node * const root) {
  if (0 == root->level) return 0;
  return 1 + std::max(node_depth(root->left), node_depth(root->right));
}

size_t log2floor(const size_t x) {
  if (x <= 1) return 0;
  return 1+log2floor(x/2);
}

size_t log2ceiling(const size_t x) {
  if (x <= 1) return 0;
  return 1+log2floor((x+1)/2);
}

void depth_test() {
  {
    dmap<char, bool> actual;
    size_t size = 0;
    for (size_t i = 0; i < 256; ++i) {
      const char key = std::rand();
      if (actual.insert(key, true).first) ++size;
      assert (node_depth(actual.root) <= std::max(static_cast<size_t>(1),2*log2ceiling(size)));
    }
  }
  {
    dmap<size_t, bool> actual;
    size_t i = 1;
    for (; i <= (1 << 10); ++i) {
      actual.insert(i, true);
      const size_t node_depth_here = node_depth(actual.root);
      const size_t expected_max_node_depth = std::max(static_cast<size_t>(1),2*log2ceiling(i));
      assert (node_depth_here <= expected_max_node_depth);
    }
  }
  {
    dmap<size_t, bool> actual;
    size_t next_maximum = 1;
    for (size_t i = 1; i <= (1 << 10); ++i) {
      actual.insert(i, true);
      const size_t node_depth_here = node_depth(actual.root);
      const size_t expected_max_node_depth = std::max(static_cast<size_t>(1),2*log2ceiling(i));
      if (i == (static_cast<size_t>(1) << next_maximum) - 2) {
	assert (node_depth_here == expected_max_node_depth);
	++next_maximum;
      }
    }
  }
}
*/

struct no_default_constructor {
  int data;
  explicit no_default_constructor(int data) : data(data) {}
  bool operator<(const no_default_constructor& that) const { return data < that.data; }
};

struct ndchash {
  size_t operator()(const no_default_constructor& x) const {
    static std::hash<int> hasher;
    return hasher(x.data);
  }
};

void compile_test() {
  base_hash_map<no_default_constructor, no_default_constructor, ndchash> actual;
  actual.insert(no_default_constructor(1), no_default_constructor(2));
  actual.find(no_default_constructor(3));
}

int main() {
  std::srand(time(NULL));
  copy_map_find_test();
  //size_test();
  //iterator_test();
  //depth_test();
  compile_test();
  //tlsf_destroy(tlsf_alloc_pool);
}
