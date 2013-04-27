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


