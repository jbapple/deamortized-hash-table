#include <atomic>
#include <mutex>
extern "C" { #include "tlsf.h" }

roots * tlsf_alloc_pool;
std::mutex pool_mutex;
