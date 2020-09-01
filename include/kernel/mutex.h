#ifndef KERNEL_MUTEX_H
#define KERNEL_MUTEX_H

#include <stdbool.h>
#include "common/mutex.h"

// The syscall
bool k_mutex(unsigned op, Mutex* m);

// The actual work
void k_init_mutex(Mutex* m);
bool k_unlock_mutex(Mutex* m);
bool k_lock_mutex(Mutex* m);

#endif /* ifdef KERNEL_MUTEX_H */
