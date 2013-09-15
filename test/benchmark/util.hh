#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <algorithm>

#include <cassert>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern "C" {
#include <papi.h>
}

using std::vector;
using std::pair;
using std::unordered_map;

struct high_priority {
  const int old_policy;
  sched_param old_sp;
  high_priority();
  ~high_priority();
};

size_t median(size_t * data, const size_t length);

vector<size_t> percentiles(const vector<size_t>& data, const vector<double>& tiers);
float float_percentiles(const vector<float>& data, const vector<double>& tiers);

//vector<size_t> draws(const size_t limit, const size_t resolution)

// get_time returns the number of CPU seconds taken by the current
// process as a floating point number. Only works on Linux. Be sure to
// link with -lrt.
size_t get_time();

template<void (*f)(const size_t)> 
vector<pair<size_t, float > > 
median_papi(const size_t limit, 
            const size_t resolution, 
            const size_t samples) {
  static const vector<double> tiers(1, 0.5);
  unordered_map<size_t, vector<float> > measurements;
  srand(0);
  for (size_t i = 0; i < samples; ++i) {
    const size_t place = rand() % resolution;
    size_t n = 0;
    if (rand()%2) {
      n = (limit * place) / resolution;
    } else {
      n = static_cast<size_t>(round(pow(static_cast<double>(limit), static_cast<double>(place)/static_cast<double>(resolution))));
    }
    long long res = 0;
    int oops = 0;
    float foo, bar, baz;
    if ((oops = PAPI_ipc(&foo, &bar, &res, &baz)) < PAPI_OK) {
      std::cerr << "PAPI fail: " << PAPI_strerror(oops) << std::endl;
      exit(1);
    }
    f(n);
    if ((oops = PAPI_ipc(&foo, &bar, &res, &baz)) < PAPI_OK) {
      std::cerr << "PAPI fail: " << PAPI_strerror(oops) << std::endl;
      exit(1);
    }
    measurements[n].push_back(bar);
  }
  vector<pair<size_t, float > > ans;
  for(const auto& i : measurements) {
    ans.push_back(std::make_pair(i.first, float_percentiles(i.second, tiers)));
  }
  sort(ans.begin(), ans.end());
  return ans;
}


template<void (*f)(const size_t)> 
vector<pair<size_t, vector<size_t> > > 
median_time(const size_t limit, 
            const size_t resolution, 
            const size_t samples) {
  static const vector<double> tiers(1, 0.5);
  unordered_map<size_t, vector<size_t> > measurements;
  srand(0);
  for (size_t i = 0; i < samples; ++i) {
    const size_t place = rand() % resolution;
    size_t n = 0;
    if (rand()%2) {
      n = (limit * place) / resolution;
    } else {
      n = static_cast<size_t>(round(pow(static_cast<double>(limit), static_cast<double>(place)/static_cast<double>(resolution))));
    }
    __sync_synchronize();
    const size_t begin = get_time(); //PAPI_get_virt_nsec(); //get_time();
    __sync_synchronize();
    f(n);
    __sync_synchronize();
    const size_t end = get_time(); //PAPI_get_virt_nsec(); //get_time();
    __sync_synchronize();
    measurements[n].push_back(end-begin);
  }
  vector<pair<size_t, vector<size_t> > > ans;
  for(const auto& i : measurements) {
    ans.push_back(make_pair(i.first, percentiles(i.second, tiers)));
  }
  sort(ans.begin(), ans.end());
  return ans;
}


// reads a T from the string argument
template<typename T>
T read(const std::string& x) {
  std::istringstream y(x);
  T ans;
  y >> ans;
  return ans;
}


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
double avg(const std::vector<T>& xs) {
  const double size = static_cast<double>(xs.size());
  T total = 0;
  for (const auto& x : xs) {
    total += x;
  }
  return static_cast<double>(total)/size;
}

template<typename T>
T maximum(const std::vector<T>& xs) {
  T ans = 0;
  for (const auto& x : xs) {
    ans = std::max(ans, x);
  }
  return ans;
}

template<typename T>
T minimum(const std::vector<T>& xs) {
  T ans = xs[0];
  for (const auto& x : xs) {
    ans = std::min(ans, x);
  }
  return ans;
}


#endif
