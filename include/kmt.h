#ifndef _KMT_H_
#define _KMT_H_

#include <kernel.h>
#include <simple_lock.h>

#define MAX_FILE_PER_THREAD 256
#define KERNEL_MAX_THREAD 1024
#define STACK_SIZE (1 << 22)
#define STACK_MAGIC 0xcc
#define THREAD_POOL_MAGIC 0xcccccccc


struct semaphore {
    char name[100];
    int count;
};

spinlock_t semaphore_lock;

#include <vfs.h>

struct thread {
    int32_t id;
    _RegSet *status;
    volatile char sleep;
    sem_t *current_waiting;
    
    kuid_t uid;
    kgid_t gid;
    
    _Area stack;
    struct fs_struct fs;
    
    struct file *file_descriptors[MAX_FILE_PER_THREAD];
};


spinlock_t thread_lock;
thread_t *thread_pool[KERNEL_MAX_THREAD + 2];
uint32_t current_thread_index;
thread_t *current;

#endif
