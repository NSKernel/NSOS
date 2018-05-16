#include <simple_lock.h>
#include <string.h>
#include <os/syslog.h>

static uint32_t lockcount = 0;
static int release_intr = 0;

void simple_lock_init(spinlock_t *lock, const char *name) {
    strncpy(lock->name, name, 100);
    lock->status = 0;
}

void simple_lock_try(spinlock_t *lock) {
    int intr;
    intr = _intr_read();
    _intr_write(0);
    if (lock->status == 1) {
        syslog("KMT SPIN", "Attempted to lock a locked lock");
    }
    else {
        if (lockcount == 0) {
            release_intr = intr;
        }
        lock->status = 1;
        lockcount += 1;
    }
}

void simple_lock_unlock(spinlock_t *lock) {
    if (lock->status == 0) {
        syslog("KMT SPIN", "Attempted to unlock an unlocked lock");
    }
    else {
        lockcount -= 1;
        lock->status = 0;
        if (lockcount == 0) {
            if (release_intr)
                _intr_write(1);
        }
    }
}
