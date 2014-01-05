#include <atomic>
#include <mutex>
extern "C" { 
#include "tlsf.h"
}

#include "tlsf_allocator.hpp"

TlsfWrapper tlsf_wrapper;
std::mutex pool_mutex;
