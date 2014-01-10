#include <set>
#include <map>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <limits>

#include <time.h>

#include "../src/deamortized_map.hh"

struct none {};
template<typename Key, typename Val> using dmap = deamortized_map<Key, Val, std::allocator<char>, std::less<Key> >;

template<typename Node>
void print_node_keys(const Node * const root) {
  if (0 == root->level) return;
  print_node_keys(root->left);
  std::cout << ' ' << static_cast<int>(root->key);
  print_node_keys(root->right);
}

template<typename Val>
void print_map_keys(const dmap<char, Val>& m) {
  print_node_keys(m.root);
  std::cout << std::endl;
}

void copy_map_find_test() {
  dmap<char, int> actual;
  std::map<char, int> expected;
  for (size_t i = 0; i < 100; ++i) {
    const char key = std::rand();
    const int val = std::rand();
    //std::cout << "key: " << static_cast<int>(key) << std::endl;
    actual.soft_insert(key, val);
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

template<typename Key, typename Val>
size_t iterator_length(const dmap<Key,Val>& m) {
  size_t ans = 0;
  auto* here = m.head;
  while (here) {
    ++ans;
    if (here->succ) assert (here->succ->pred == here);
    here = here->succ;
  }
  return ans;
}

template<typename A, typename E>
bool iterator_match(const A& actual, const E& expected) {
  auto e = expected.begin();
  auto* a = actual.head;
  while (a) {
    if (a->succ) assert (a->succ->pred == a);
    assert(*e == a->key);
    assert (e != expected.end());
    a = a->succ;
    ++e;
  }
  return e == expected.end();
}

void iterator_test() {
  dmap<char, bool> actual;
  std::set<char> expected;
  size_t size = 0;
  for (size_t i = 0; i < 100; ++i) {
    const char key = std::rand();
    if (actual.soft_insert(key, true).first) ++size;
    expected.insert(key);
    assert (iterator_match(actual, expected));
    //assert (iterator_length(actual) == size);
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
      if (actual.soft_insert(key, true).first) ++size;
      assert (node_depth(actual.root) <= std::max(static_cast<size_t>(1),2*log2ceiling(size)));
    }
  }
  {
    dmap<size_t, bool> actual;
    size_t i = 1;
    for (; i <= (1 << 10); ++i) {
      actual.soft_insert(i, true);
      const size_t node_depth_here = node_depth(actual.root);
      const size_t expected_max_node_depth = std::max(static_cast<size_t>(1),2*log2ceiling(i));
      assert (node_depth_here <= expected_max_node_depth);
    }
  }
  {
    dmap<size_t, bool> actual;
    size_t next_maximum = 1;
    for (size_t i = 1; i <= (1 << 10); ++i) {
      actual.soft_insert(i, true);
      const size_t node_depth_here = node_depth(actual.root);
      const size_t expected_max_node_depth = std::max(static_cast<size_t>(1),2*log2ceiling(i));
      if (i == (static_cast<size_t>(1) << next_maximum) - 2) {
	assert (node_depth_here == expected_max_node_depth);
	++next_maximum;
      }
    }
  }
}

struct no_default_constructor {
  int data;
  explicit no_default_constructor(int data) : data(data) {}
  bool operator<(const no_default_constructor& that) const { return data < that.data; }
};

void compile_test() {
  dmap<no_default_constructor, no_default_constructor> actual;
  actual.soft_insert(no_default_constructor(1), no_default_constructor(2));
  actual.find(no_default_constructor(3));
}

int main() {
  std::srand(time(NULL));
  copy_map_find_test();
  //size_test();
  iterator_test();
  depth_test();
  compile_test();
  //tlsf_destroy(tlsf_alloc_pool);
}
