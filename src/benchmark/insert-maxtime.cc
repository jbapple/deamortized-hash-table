#include "util.hh"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include "linear-probing.cc"
#include "lazy-linear.cc"

#include <sched.h>



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
  bool operator<(const some& that) const {
    for (int i = 0; i < some::some_size; ++i) {
      if (x[i] < that.x[i]) return true;
      if (x[i] > that.x[i]) return false;
    }
    return false;
  }

};

size_t hashf(const some & x) {
  size_t ans;
  for (int i = 0; i < some::some_size; ++i) {
    ans ^= x.x[i];
  }
  return ans;
}

size_t hash(const some & x) {
  return hashf(x);
}

typedef some sample_type;

template<typename T>
std::vector<std::pair<unsigned, double> > test(const unsigned size, const unsigned samples) {
  unsigned i = 0;
  std::vector<std::pair<unsigned, double> > ans;//(size * samples); 
  //high_priority zz;
  for (unsigned k = 0; k < samples; ++k) { 
    T playground;
    double leader = 0.0;
    for (unsigned j = 0; j < size; ++j) { 
      const auto start = get_time();
      playground.insert(some::random());
      const auto here = get_time() - start; 
      leader = std::max(leader, here);
      ans.push_back(make_pair(j,leader));
    } 
  }
  return ans;
}

void print_test(std::vector<std::pair<unsigned, double> > x) {
  std::unordered_map<unsigned, std::vector<double> > collect;    
  for (const auto& y : x) {
    collect[y.first].push_back(y.second);
  }
  for (const auto& y : collect) {
    std::cout << y.first << '\t' << avg(y.second) << std::endl;
  }
}

int main(int argc, char ** argv) {
  const unsigned size = 10000;
  const unsigned samples = 10;
  test<quiet_map<sample_type, lazier_map<sample_type, BasicBitArray> > >(size, samples);
}
