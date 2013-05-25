#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <set>

#include <cassert>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// reads a T from the string argument
template<typename T>
T read(const std::string& x) {
  std::istringstream y(x);
  T ans;
  y >> ans;
  return ans;
}

// get_time returns the number of CPU seconds taken by the current
// process as a floating point number. Only works on Linux. Be sure to
// link with -lrt.
double get_time();

// unique_pseudo_random_bytes(n) returns n unique values from a
// pseudo-random number generator for a POD type T
template<typename T>
std::vector<T> unique_pseudo_random_bytes(const size_t n) {
  static const int fd = open("/dev/urandom", O_RDONLY);
  assert (fd != -1);
  std::vector<T> ans(n);
  T samp = 0;
  std::unordered_set<T> collect;
  for (size_t count = 0; count < n; ++count) {
    while (collect.find(samp) != collect.end()) {
      while (sizeof(T) != read(fd, &samp, sizeof(T)));
    }
    ans[count] = samp;
    collect.insert(samp);
  }
  return ans;
}

template<typename T>
T avg(const std::vector<T>& xs) {
  const T size = static_cast<T>(xs.size());
  T total = 0;
  for (const auto& x : xs) {
    total += x;
  }
  return total/size;
}

template<typename T>
T maximum(const std::vector<T>& xs) {
  T ans = 0;
  for (const auto& x : xs) {
    ans = std::max(ans, x);
  }
  return ans;
}


#endif
