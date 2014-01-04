#include <stddef.h>

struct roots;

// TODO: give memory back to the OS

struct roots * tlsf_create(void * (*allocate)(size_t), void (*deallocate)(void *, size_t));
void * tlsf_malloc(struct roots *, size_t);
void tlsf_free(struct roots *, void *);
void tlsf_destroy(struct roots *);


