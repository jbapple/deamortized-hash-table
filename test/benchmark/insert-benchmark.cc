#include "util.hh"

#include <set>

typedef int sample_type;

template<typename T>
std::vector<std::pair<unsigned, double> > test(const unsigned size, const unsigned samples) {
  const auto to_insert = unique_pseudo_random_bytes<sample_type>((size * (size+1) / 2) * samples);
  unsigned i = 0;
  unsigned m = 0;
  std::vector<std::pair<unsigned, double> > ans(size*samples);
  for (unsigned k = 0; k < samples; ++k) {
    for (unsigned j = 0; j < size; ++j) {
      const auto start = get_time();
      T playground;
      for (unsigned m = 0; m < j; ++m) {
        playground.insert(to_insert[i++]);
      }
      ans[m++] = std::make_pair(j, get_time() - start);
    }
  } 
  return ans;
}

void print_test(std::vector<std::pair<unsigned, double> > x) {
  for (const auto& y : x) {
    std::cout << y.first << '\t' << y.second << std::endl;
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
