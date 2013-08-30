#include "util.hh"

#include "time.h"

// get_time returns the number of CPU seconds taken by the current
// process as a floating point number. Be sure to link with -lrt.
size_t get_time() {
  static const size_t freq = 1000000000ull;
  static timespec atime;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &atime);
  return (static_cast<size_t>(atime.tv_sec) * freq) + static_cast<size_t>(atime.tv_nsec);
}


high_priority::high_priority() : old_policy(sched_getscheduler(0)), old_sp() {
  sched_getparam(0, &old_sp);
  sched_param sp;
  sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if (-1 == sched_setscheduler(0, SCHED_FIFO, &sp)) {
    std::cerr << "can't set realtime" << std::endl;
    //throw "can't set realtime";
    //exit(1);
  }
  
  cpu_set_t aff;
  CPU_ZERO(&aff);
  CPU_SET(0, &aff);
  if (false && (-1 == sched_setaffinity(0, sizeof(cpu_set_t), &aff))) {
    std::cerr << "can't set affinity" << std::endl;
    //throw "can't set affinity";
    //exit(1);
  }
  
}

high_priority::~high_priority() {
  sched_setscheduler(0, old_policy, &old_sp);
}


size_t median(size_t * data, const size_t length) {
  std::sort(data, data + length);
  if (length & 1) {
    return 2*data[length/2];
  }
  return data[length/2] + data[length/2 - 1];
}

vector<size_t> percentiles(const vector<size_t>& data, const vector<double>& ) {
  auto foo = data;
  double ans = 0;
  sort(foo.begin(), foo.end());
  for (auto i : foo)  ans += i;
  return vector<size_t>(1,ans/foo.size());//foo.front());//[foo.size() * 0.99]);//foo[foo.size()/2]);
}
