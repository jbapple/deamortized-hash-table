#include "deamortized_hash_set.hh"

int main() {
  DeamortizedHashSet<int> foo;
  for (int i = 0; i < 1000000; ++i) {
    foo.insert(rand());
  }
  for (int i = 0; i < RAND_MAX; ++i) {
    foo.erase(i);
  }
}

