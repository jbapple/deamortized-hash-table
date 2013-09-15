#include "deamortized_hash_set.hh"

int main() {
  DeamortizedHashSet<int> foo;
  for (int i = 0; i < 1000000; ++i) {
    foo.insert(rand() & (0xffff));
  }
  for (int i = 0; i < 0xffff; ++i) {
    foo.erase(i);
  }
}

