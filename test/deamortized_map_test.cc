#include <map>
#include <cstdlib>
#include <cassert>
#include <iostream>

#include <time.h>

#include "../src/deamortized_map.hh"

void print_node_keys(const typename deamortized_map<char, int>::Node * const root) {
  if (0 == root->level) return;
  print_node_keys(root->left);
  std::cout << ' ' << static_cast<int>(root->key);
  print_node_keys(root->right);
}

void print_map_keys(const deamortized_map<char, int>& m) {
  print_node_keys(m.root);
  std::cout << std::endl;
}

void copy_map_find_test() {
  deamortized_map<char, int> actual;
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

void size_test() {
  deamortized_map<char, bool> actual;
  size_t expected = 0;
  for (size_t i = 0; i < 10000; ++i) {
    const char key = std::rand();
    if (NULL == actual.find(key)) ++expected;
    actual.insert(key, true);
    assert (expected == actual.size);
  }
}

template<typename Key, typename Val>
size_t iterator_length(const deamortized_map<Key,Val>& m) {
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
  deamortized_map<char, bool> actual;
  for (size_t i = 0; i < 100; ++i) {
    const char key = std::rand();
    actual.insert(key, true);
    assert (iterator_length(actual) == actual.size);
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
  if (0 == x) return 0;
  return 1+log2floor((x+1)/2);
}

void depth_test() {
  {
    deamortized_map<char, bool> actual;
    for (size_t i = 0; i < 256; ++i) {
      const char key = std::rand();
      actual.insert(key, true);
      //std::cout << node_depth(actual.root) << " <= " << 2*log2ceiling(actual.size) << std::endl;
      assert (node_depth(actual.root) <= 2*log2ceiling(actual.size));
    }
  }
  {
    deamortized_map<size_t, bool> actual;
    for (size_t i = 0; i < 256; ++i) {
      actual.insert(i, true);
      //std::cout << node_depth(actual.root) << " <= " << 2*log2ceiling(actual.size) << std::endl;
      assert (node_depth(actual.root) <= 2*log2ceiling(actual.size));
    }
  }
}

struct no_default_constructor {
  int data;
  no_default_constructor(int data) : data(data) {}
  bool operator<(const no_default_constructor& that) const { return data < that.data; }
};

void compile_test() {
  deamortized_map<no_default_constructor, no_default_constructor> actual;
  actual.insert(no_default_constructor(1), no_default_constructor(2));
  actual.find(no_default_constructor(3));
}

int main() {
  std::srand(time(NULL));
  copy_map_find_test();
  size_test();
  iterator_test();
  depth_test();
  compile_test();
}

