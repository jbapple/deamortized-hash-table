#include <vector>
#include <cstdlib>

using namespace std;

#include "lazy-linear.cc"

struct some {
  const static int some_size = 2;
  int x[some_size];
  static some random() {
    some ans;
    for (int i = 0; i < some::some_size; ++i) {
      ans.x[i] = rand();
    }
    return ans;
  }
  bool operator==(const some& that) const {
    for (int i = 0; i < some::some_size; ++i) {
      if (x[i] != that.x[i]) return false;
    }
    return true;
  }
};

size_t hashf(const some & x) {
  size_t ans;
  for (int i = 0; i < some::some_size; ++i) {
    ans ^= x.x[i];
  }
  return ans;
}

typedef some sample_type;

template<typename T>
void test(const unsigned size, const unsigned samples) {
  for (unsigned k = 0; k < samples; ++k) { 
    T playground;
    for (unsigned j = 0; j < size; ++j) { 
      playground.insert(some::random());
    } 
  }
}

int main() {
  srand(0);
  const unsigned size = 10000;
  const unsigned samples = 10;
  test<quiet_map<sample_type, lazier_map<sample_type, BasicBitArray> > >(size, samples);
}
