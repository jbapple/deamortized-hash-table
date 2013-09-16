#include "deamortized_hash_set.hh"

int main() {
  for (int j = 0; j < 30; ++j) {
    DeamortizedHashSet<int> foo;
    srand(0);
    for (int i = 0; i < (1 << j); ++i) {
      foo.insert(rand());
    }
    srand(0);
    for (int i = 0; i < (1 << j); ++i) {
      foo.erase(rand());
    }
  }
}

