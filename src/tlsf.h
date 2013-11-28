#include <stddef.h>

struct roots;

// TODO: give memory back to the OS

struct roots * tlsf_init_from_block(void * begin, size_t length);
void tlsf_add_block(struct roots *, void * begin, size_t length);
void * tlsf_malloc(struct roots *, size_t);
void tlsf_free(struct roots *, void *);
size_t tlsf_get_capacity(struct roots *);
extern const size_t tlsf_padding;

