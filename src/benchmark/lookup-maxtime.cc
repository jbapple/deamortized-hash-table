#include "util.hh"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include "src/linear-probing.cc"

typedef int sample_type;

template<typename T>
std::vector<std::pair<unsigned, double> > test(const unsigned size, const unsigned samples) {
  char * stupid = 0;
  unsigned i = 0;
  std::vector<std::pair<unsigned, double> > ans(size);// * samples); 
  const int old_policy = sched_getscheduler(0);
  sched_param sp, old_sp;
  sched_getparam(0, &old_sp);
  sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if (-1 == sched_setscheduler(0, SCHED_FIFO, &sp)) {
    std::cerr << "can't set realtime" << std::endl;
    exit(1);
  }
  cpu_set_t aff;
  CPU_ZERO(&aff);
  CPU_SET(0, &aff);
  if (-1 == sched_setaffinity(0, sizeof(cpu_set_t), &aff)) {
    std::cerr << "can't set affinity" << std::endl;
    exit(1);
  }
  std::vector<T> playground(samples);
  double leader = 0.0;
  for (unsigned j = 0; j < size; ++j) { 
    const auto to_insert = rand();
    const auto to_lookup = rand();
    for (unsigned k = 0; k < samples; ++k) { 
      playground[k].insert(to_insert);
    }
    const auto start = get_time();
    for (unsigned k = 0; k < samples; ++k) { 
      stupid += reinterpret_cast<size_t>(&*playground[k].find(to_lookup));
    }
    const auto here = get_time() - start; 
    leader = here; //std::max(leader, here);
    ans[j] = std::make_pair(j, leader/samples);
     
  }  
  sched_setscheduler(0, old_policy, &old_sp);
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
