#include <os.h>
#include <kmt.h>
#include <string.h>
#include <simple_lock.h>
#include <os/syslog.h>

void kmt_init();
int kmt_create(thread_t *thread, void (*entry)(void *arg), void *arg);
void kmt_teardown(thread_t *thread);
thread_t *kmt_schedule();
void kmt_spin_init(spinlock_t *lk, const char *name);
void kmt_spin_lock(spinlock_t *lk);
void kmt_spin_unlock(spinlock_t *lk);
void kmt_sem_init(sem_t *sem, const char *name, int value);
void kmt_sem_wait(sem_t *sem);
void kmt_sem_signal(sem_t *sem);

MOD_DEF(kmt) {
    .init = kmt_init,
    .create = kmt_create,
    .schedule = kmt_schedule,
    .teardown = kmt_teardown,
    .spin_init = kmt_spin_init,
    .spin_lock = kmt_spin_lock,
    .spin_unlock = kmt_spin_unlock,
    .sem_init = kmt_sem_init,
    .sem_wait = kmt_sem_wait,
    .sem_signal = kmt_sem_signal
};

void kmt_init() {
    syslog("KMT", "Initializing KMT...");
    uint32_t i;
    for (i = 1; i <= KERNEL_MAX_THREAD; i++) {
        thread_pool[i] = NULL;
    }
    current_thread_index = -1;
    
    kmt_spin_init(&thread_lock, "kernel thread lock");
    kmt_spin_init(&semaphore_lock, "semaphore lock");
    
    thread_pool[0] = (thread_t*)THREAD_POOL_MAGIC;
    thread_pool[KERNEL_MAX_THREAD + 1] = (thread_t*)THREAD_POOL_MAGIC;
    
    syslog("KMT", "KMT initialization is done.");
}

int kmt_create(thread_t *thread, void (*entry)(void *arg), void *arg) {
    kmt_spin_lock(&thread_lock);
    int i;
    
    for (i = 1; i <= KERNEL_MAX_THREAD; i++) {
        if (thread_pool[i] == NULL) {
            break;
        }
    }
    if (i > KERNEL_MAX_THREAD) {
        syslog("KMT", "Attempted to create a thread while the thread pool is full");
        
        kmt_spin_unlock(&thread_lock);
        return -1;
    }
    
    thread_pool[i] = thread;
    
    thread->id = i;
    thread->stack.start = pmm->alloc(STACK_SIZE + 2 * sizeof(uint8_t)) + sizeof(uint8_t);
    thread->stack.end = thread->stack.start + STACK_SIZE;
    *(uint8_t*)(thread->stack.start - sizeof(uint8_t)) = STACK_MAGIC; // fence
    *((uint8_t*)thread->stack.end) = STACK_MAGIC;
    thread->sleep = 0;
    thread->current_waiting = NULL;
    thread->status = _make(thread->stack, entry, arg);
    thread->uid = 0; // kernel threads are root!
    thread->gid = 0; // kernel threads are root!
    
    thread->fs.root = current->fs.root;
    thread->fs.pwd = current->fs.pwd;
    thread->fs.altroot = current->fs.altroot;
    thread->fs.rootmnt = current->fs.rootmnt;
    thread->fs.pwdmnt = current->fs.pwdmnt;
    thread->fs.altrootmnt = current->fs.altrootmnt;
    
    
    for (i = 3; i < MAX_FILE_PER_THREAD; i++) {
        thread->file_descriptors[i] = NULL;
    }
    thread->file_descriptors[STDIN_FILENO] = NULL + 1; // can be anything but not NULL
    thread->file_descriptors[STDOUT_FILENO] = NULL + 1; // can be anything but not NULL
    thread->file_descriptors[STDERR_FILENO] = NULL + 1; // can be anything but not NULL
    
    syslog("KMT", "Created kernel thread with stack from 0x%08X to 0x%08X", thread->stack.start, thread->stack.end);
    
    kmt_spin_unlock(&thread_lock);
    return 0;
}

void kmt_teardown(thread_t *thread) {
    kmt_spin_lock(&thread_lock);
    
    uint32_t i;
    for (i = 1; i <= KERNEL_MAX_THREAD; i++) {
        if (thread_pool[i] == thread)
            break;
    }
    if (i > KERNEL_MAX_THREAD) {
        syslog("KMT", "Attempted to kill a non-exist thread");
        return;
    }
    if (i == current_thread_index) {
        syslog("KMT", "FATAL: A thread commited suicide. Kernel thread stack corrupted.");
        panic("Kernel thread stack corrupted");
    }
    
    // stack integrety check
    if (*(uint8_t*)(thread_pool[i]->stack.start - sizeof(uint8_t)) != STACK_MAGIC ||
        *(uint8_t*)(thread_pool[i]->stack.end) != STACK_MAGIC ) {
        syslog("KMT", "FATAL: Kernel thread stack corrupted.");
        panic("Kernel thread stack corrupted");
    }
    
    pmm->free(thread_pool[i]->stack.start - sizeof(uint8_t));
    thread_pool[i] = NULL;
    
    kmt_spin_unlock(&thread_lock);
}

thread_t *kmt_schedule() {
    uint32_t i = current_thread_index;
    
    if (current_thread_index != -1) {
        // check thread pool integrety
        if ((uint32_t)(thread_pool[0]) != THREAD_POOL_MAGIC ||
            (uint32_t)(thread_pool[KERNEL_MAX_THREAD + 1]) != THREAD_POOL_MAGIC) {
            syslog("KMT", "FATAL: Kernel thread pool corrupted.");
            panic("Kernel thread pool corrupted");
        }
        
        // check current stack integrety
        if (*(uint8_t*)(thread_pool[current_thread_index]->stack.start - sizeof(uint8_t)) != STACK_MAGIC ||
            *(uint8_t*)(thread_pool[current_thread_index]->stack.end) != STACK_MAGIC) {
            syslog("KMT", "FATAL: Kernel thread %d stack from 0x%08X to 0x%08X corrupted.", current_thread_index, thread_pool[current_thread_index]->stack.start, thread_pool[current_thread_index]->stack.end);
            panic("Kernel thread stack corrupted");
        }
        
    }
    
    
    do {
        i %= KERNEL_MAX_THREAD;
        i += 1;
    } while (thread_pool[i] == NULL || thread_pool[i]->sleep != 0);
    current_thread_index = i;
    
    return thread_pool[i];
}

void kmt_spin_init(spinlock_t *lk, const char *name) {
    simple_lock_init(lk, name);
}

void kmt_spin_lock(spinlock_t *lk) {
    simple_lock_try(lk);
}

void kmt_spin_unlock(spinlock_t *lk) {
    simple_lock_unlock(lk);
}

void kmt_sem_init(sem_t *sem, const char *name, int value) {
    strncpy(sem->name, name, 100);
    sem->count = value;
}

void kmt_sem_wait(sem_t *sem) {
    kmt_spin_lock(&semaphore_lock);
    while (sem->count == 0) {
        thread_pool[current_thread_index]->current_waiting = sem;
        thread_pool[current_thread_index]->sleep = 1;
        kmt_spin_unlock(&semaphore_lock);
        while (thread_pool[current_thread_index]->sleep != 0);
        //syslog("KMT", "Tries to lock...");
        kmt_spin_lock(&semaphore_lock);
    }
    sem->count -= 1;
    kmt_spin_unlock(&semaphore_lock);
}

void kmt_sem_signal(sem_t *sem) {
    uint32_t threadit;

    kmt_spin_lock(&semaphore_lock);
    sem->count += 1;
    
    for (threadit = 1; threadit <= KERNEL_MAX_THREAD; threadit++) {
        if (thread_pool[threadit]->sleep != 0 && thread_pool[threadit]->current_waiting == sem) {
            break;
        }
    }
    if (threadit <= KERNEL_MAX_THREAD) {
        thread_pool[threadit]->sleep = 0;
        thread_pool[threadit]->current_waiting = NULL;
    }
    
    kmt_spin_unlock(&semaphore_lock);
}
