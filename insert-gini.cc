#include "util.hh"

#include <set>
#include <algorithm>

typedef int sample_type;

template<typename T>
std::vector<std::vector<double> > test(const unsigned size, const unsigned samples) {
  const auto to_insert = unique_pseudo_random_bytes<sample_type>(size * samples);
  unsigned i = 0;
  std::vector<std::vector<double> > ans;
  for (unsigned k = 0; k < samples; ++k) {
    ans.push_back(std::vector<double>());
    T playground;
    for (unsigned j = 0; j < size; ++j) {
      const auto start = get_time();
      playground.insert(to_insert[i++]);
      ans.back().push_back(get_time() - start);
    }
    double sum = 0.0;
    for (auto time : ans.back()) {
      sum += time;
    }
    for (auto & time : ans.back()) {
      time /= sum;
      time *= 100; // percent
    }
    std::sort(ans.back().begin(), ans.back().end());
  } 
  return ans;
}

void print_test(std::vector<std::vector<double> > x) {
  for (int i = 0; i < x.size(); i += 2) {
    for (int j = 0; j < x[i].size(); ++j) {
      std::cout << (x[i].size() - j) << '\t' << x[i][j] << std::endl;
    }
    for (int j = 0; j < x[i+1].size(); ++j) {
      std::cout << j+1 << '\t' << x[i+1][x[i+1].size() - j - 1] << std::endl;
    }
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
