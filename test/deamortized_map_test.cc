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

int main() {
  std::srand(time(NULL));
  deamortized_map<char, int> actual;
  std::map<char, int> expected;
  for (size_t i = 0; i < 100; ++i) {
    const char key = std::rand();
    const int val = std::rand();
    std::cout << "key: " << static_cast<int>(key) << std::endl;
    actual.insert(key, val);
    auto j = expected.find(key);
    if (j == expected.end()) expected.insert(std::make_pair(key, val));
    print_map_keys(actual);
    const char tester = std::rand();
    std::cout << "tester: " << static_cast<int>(tester)
	      << std::boolalpha
	      << '\t' << (actual.find(tester) == NULL)
	      << '\t' << (expected.find(tester) == expected.end())
	      << std::endl;
    assert ((actual.find(tester) == NULL) == (expected.find(tester) == expected.end()));
  }
}
