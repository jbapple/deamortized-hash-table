#include <stddef.h>

struct roots;

struct roots * init_tlsf_from_malloc(const size_t size);
struct roots * init_tlsf_from_block(void * begin, const size_t length);
void * tlsf_malloc(struct roots * const, const size_t);
void tlsf_free(struct roots * const, void * const);
