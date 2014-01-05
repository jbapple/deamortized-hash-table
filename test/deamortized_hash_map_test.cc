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

template<typename K, typename V>
void print_bhm(const base_hash_map<K,V>& x) {
  auto i = x.here->live_head;
  while (i) {
    std::cout << i->key << ' ';
    i = i->live_next;
  }
  std::cout << std::endl;
}

template<typename K, typename V>
void print_sm(const std::map<K,V>& x) {
  for(auto i : x) {
    std::cout << i.first << ' ';
  }
  std::cout << std::endl;
}

void resize_test() {
  base_hash_map<size_t, bool, std::hash<size_t>, std::allocator<char> > actual;
  std::map<size_t, bool> expected;
  const size_t limit = ((size_t)1) << 12;
  for (size_t i = 0; i < limit; ++i) {
    const size_t key = std::rand() & (limit-1);
    actual.insert(key, true);
    expected.insert(std::make_pair(key, true));
    const size_t tester = std::rand() & (limit-1);
    assert ((actual.find(tester) == NULL) == (expected.find(tester) == expected.end()));
  }
  //print_bhm(actual);
  //print_sm(expected);
  for (size_t i = 0; i < 24 * limit; ++i) {
    const size_t key = std::rand() & (limit-1);
    //std::cout << "erasing: " << key << std::endl;
    actual.erase(actual.find(key));
    expected.erase(key);
    //print_bhm(actual);
    //print_sm(expected);
    const size_t tester = std::rand() & (limit-1);
    //std::cout << "testing: " << tester << std::endl;
    assert ((actual.find(tester) == NULL) == (expected.find(tester) == expected.end()));
  }

}

template<typename T>
size_t iterator_length(const T& m) {
  size_t ans = 0;
  auto* here = m.here->live_head;
  while (here) {
    ++ans;
    if (here->live_next) assert (here->live_next->live_prev == here);
    here = here->live_next;
  }
  return ans;
}


void iterator_test() {
  base_hash_map<char, bool> actual;
  size_t size = 0;
  for (size_t i = 0; i < 256; ++i) {
    const char key = std::rand();
    if (actual.insert(key, true).first) ++size;
    if (std::rand() % 2) {
      const char key = std::rand();
      if (actual.erase(actual.find(key))) --size;
    }
    assert (iterator_length(actual) == size);
    assert (size == actual.here->node_count - actual.here->tombstone_count);
  }  
  for (size_t i = 0; i < 256 * 16; ++i) {
    const char key = std::rand();
    if (actual.erase(actual.find(key))) --size;
    assert (iterator_length(actual) == size);
    assert (size == actual.here->node_count - actual.here->tombstone_count);
  }  
}

/*
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
    static const std::hash<int> hasher = std::hash<int>();
    return hasher(x.data);
  }
};

void compile_test() {
  base_hash_map<no_default_constructor, no_default_constructor, ndchash> actual;
  actual.insert(no_default_constructor(1), no_default_constructor(2));
  actual.find(no_default_constructor(3));
}

int main() {
  auto seed = time(NULL);
  //seed = 1388903376;
  //seed = 1388904916;
  //seed = 1388959970;
  std::cout << "seed: " << seed << std::endl;
  std::srand(seed);
  copy_map_find_test();
  resize_test();
  //size_test();
  iterator_test();
  //depth_test();
  compile_test();
  //tlsf_destroy(tlsf_alloc_pool);
}
