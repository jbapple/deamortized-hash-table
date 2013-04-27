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

void print_test(const std::vector<std::vector<double> >& x) {
  for (int i = 0; i < x.size(); i += 2) {
    for (int j = 0; j < x[i].size(); ++j) {
      std::cout << (x[i].size() - j) << '\t' << x[i][j] << std::endl;
    }
    for (int j = 0; j < x[i+1].size(); ++j) {
      std::cout << j+1 << '\t' << x[i+1][x[i+1].size() - j - 1] << std::endl;
    }
  }
}

void print_avg(const std::vector<std::vector<double> >& x) {
  std::vector<double> ans(x[0].size(), 0.0);
  for (const auto& y : x) {
    for (int i = 0; i < y.size(); ++i) {
      ans[i] += y[i];
    }
  }
  for (int i = 0; i < ans.size(); ++i) {
    std::cout << (ans.size() - i) << '\t' << ans[i]/x.size() << std::endl;
  }
}

int main(int argc, char ** argv) {
  assert (5 == argc);
  const auto size = read<unsigned>(argv[3]);
  const auto samples = read<unsigned>(argv[4]);
  const std::string container_type_string = argv[2]; 
  const std::string plot_type = argv[1];
 
  void (*f)(const std::vector<std::vector<double> >&) = nullptr;
    
  if ("all" == plot_type) {
    f = print_test;
  } else if ("avg" == plot_type) {
    f = print_avg;
  }
 
  if ("std::set" == container_type_string) {
    f(test<std::set<sample_type> >(size, samples));
  } else if ("std::unordered_set" == container_type_string) {
    f(test<std::unordered_set<sample_type> >(size, samples));
  }

}
