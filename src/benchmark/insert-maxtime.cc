#include "util.hh"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include "linear-probing.cc"
#include "lazy-linear.cc"

#include <sched.h>

typedef int sample_type;

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
      playground.insert(rand());
      const auto here = get_time() - start; 
      leader = std::max(leader, here);
      ans.push_back(std::make_pair(j, leader));
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
  unsigned size = 100000;//00;
  unsigned samples = 1000;
  if (4 == argc) {
    size = read<unsigned>(argv[2]);
    samples = read<unsigned>(argv[3]);
  }
  const std::string container_type_string = argv[1];
  
  if ("tree" == container_type_string) {
    print_test(test<std::set<sample_type> >(size, samples));
  } else if ("std::unordered_set" == container_type_string) {
    //print_test(test<std::unordered_set<sample_type> >(size, samples));
  } else if ("hash" == container_type_string) {
    print_test(test<hash_map<sample_type> >(size, samples));
  } else if ("lazy" == container_type_string) {
    print_test(test<quiet_map<sample_type> >(size, samples));
  } else {
    std::cerr << "No test";
  }

}
