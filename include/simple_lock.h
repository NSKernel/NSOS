#ifndef _SIMPLE_LOCK_H_
#define _SIMPLE_LOCK_H_

#include <os.h>

struct spinlock {
    char name[100];
    char status;
};

void simple_lock_init(spinlock_t *lock, const char *name);
void simple_lock_try(spinlock_t *lock);
void simple_lock_unlock(spinlock_t *lock);

#endif
