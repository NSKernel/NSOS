#include <os.h>
#include <os/syslog.h>
#include <stdio.h>
#include <kmt.h>

static thread_t t[4];
volatile static char thread_complete[4];


void threadfunc(void *id) {
    int i = 0;
    while (i++ < 100)
        printf("Ring from %d\n", id);
    syslog("KMT_TEST", "Thread %d complete", (int)id);
    thread_complete[(int)id] = 1;
    while (1);
}


void kmt_test() {
    syslog("KMT_TEST", "Now performing KMT test...");
    
    int count = 0;
    
    syslog("KMT_TEST", "Creating 4 threads...");
    thread_complete[0] = thread_complete[1] = thread_complete[2] = thread_complete[3] = 0;
    while (count < 4) {
        kmt->create(&t[count], threadfunc, (void*)count);
        count++;
    }
    
    while (!(thread_complete[0] && thread_complete[1] && thread_complete[2] && thread_complete[3]));
    syslog("KMT_TEST", "All threads completed. Killing all threads");
    
    kmt->teardown(&(t[0]));
    kmt->teardown(&(t[3]));
    kmt->teardown(&(t[1]));
    kmt->teardown(&(t[2]));
    syslog("KMT_TEST", "All threads are killed");
}
