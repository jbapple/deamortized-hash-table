#include "util.hh"

#include <unordered_map>
#include <unordered_set>
#include <set>


typedef int sample_type;

template<typename T>
std::vector<std::pair<unsigned, double> > test(const unsigned size, const unsigned samples) {
  const auto to_insert = unique_pseudo_random_bytes<sample_type>((size * (size+1) / 2) * samples);
  unsigned i = 0;
  std::vector<std::pair<unsigned, double> > ans(size * samples);
  for (unsigned k = 0; k < samples; ++k) {
    const auto start = get_time();
    T playground;
    for (unsigned j = 0; j < size; ++j) { 
      playground.insert(to_insert[j]);
      ans[j] = std::make_pair(j, get_time() - start);
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
  assert (4 == argc);
  const auto size = read<unsigned>(argv[2]);
  const auto samples = read<unsigned>(argv[3]);
  const std::string container_type_string = argv[1];
  
  if ("std::set" == container_type_string) {
    print_test(test<std::set<sample_type> >(size, samples));
  } else if ("std::unordered_set" == container_type_string) {
    print_test(test<std::unordered_set<sample_type> >(size, samples));
  }

}
