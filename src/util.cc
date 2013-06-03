#include "util.hh"

#include "time.h"

// get_time returns the number of CPU seconds taken by the current
// process as a floating point number. Be sure to link with -lrt.
double get_time() {
  static const double freq = static_cast<double>(1000*1000*1000);
  static timespec atime;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &atime);
  return ((static_cast<double>(atime.tv_sec) * 1000 * 1000 * 1000) + static_cast<double>(atime.tv_nsec))/freq;
}


high_priority::high_priority() : old_policy(sched_getscheduler(0)), old_sp() {
  sched_getparam(0, &old_sp);
  sched_param sp;
  sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if (-1 == sched_setscheduler(0, SCHED_FIFO, &sp)) {
    throw "can't set realtime";
    //std::cerr << "can't set realtime" << std::endl;
    //exit(1);
  }
  /*
  cpu_set_t aff;
  CPU_ZERO(&aff);
  CPU_SET(0, &aff);
  if (-1 == sched_setaffinity(0, sizeof(cpu_set_t), &aff)) {
    throw "can't set affinity";
    //std::cerr << "can't set affinity" << std::endl;
    //exit(1);
  }
  */
}

high_priority::~high_priority() {
  sched_setscheduler(0, old_policy, &old_sp);
}
