#ifndef _PMM_H_
#define _PMM_H_

#include <kernel.h>
#include <kmt.h>

spinlock_t kernel_memory_lock;

struct memory_block {
    struct memory_block *front;
    char inuse;
    struct memory_block *next;
};

struct memory_block *memory_head;

#endif
