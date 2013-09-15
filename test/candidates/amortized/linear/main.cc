#include "deamortized_hash_set.hh"

int main() {
  DeamortizedHashSet<int> foo;
  for (int i = 0; i < 100; ++i) {
    foo.insert(i);
  }
}

