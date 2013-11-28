#include <atomic>
extern "C" { #include "tlsf.h" }

std::atomic<roots *> tlsf_alloc_pool;
