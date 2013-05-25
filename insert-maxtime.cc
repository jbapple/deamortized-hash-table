#include "util.hh"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include "src/linear-probing.cc"

#include <sched.h>

typedef int sample_type;

template<typename T>
std::vector<std::pair<unsigned, double> > test(const unsigned size, const unsigned samples) {
  unsigned i = 0;
  std::vector<std::pair<unsigned, double> > ans(size * samples); 
  const int old_policy = sched_getscheduler(0);
  sched_param sp, old_sp;
  sched_getparam(0, &old_sp);
  sp.sched_priority = sched_get_priorty_max(SCHED_FIFO);
  set_scheduler(0, SCHED_FIFO, &sp);
  for (unsigned k = 0; k < samples; ++k) { 
    T playground;
    double leader = 0.0;
    for (unsigned j = 0; j < size; ++j) { 
      const auto start = get_time();
      playground.insert(rand());
      const auto here = get_time() - start; 
      leader = std::max(leader, here);
      ans[j] = std::make_pair(j, leader);
    } 
  }
  set_scheduler(0, old_policy, &old_sp)
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
  assert (4 == argc);
  const auto size = read<unsigned>(argv[2]);
  const auto samples = read<unsigned>(argv[3]);
  const std::string container_type_string = argv[1];
  
  if ("std::set" == container_type_string) {
    print_test(test<std::set<sample_type> >(size, samples));
  } else if ("std::unordered_set" == container_type_string) {
    print_test(test<std::unordered_set<sample_type> >(size, samples));
  } else if ("linear-probing" == container_type_string) {
    print_test(test<hash_map<sample_type> >(size, samples));
  }else {
    std::cerr << "No test";
  }

}
