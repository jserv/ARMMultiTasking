#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);
void free_all(int tid);

#endif /* ifdef ALLOC_H */
