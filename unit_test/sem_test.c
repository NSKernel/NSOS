#include <os.h>
#include <os/syslog.h>
#include <stdio.h>
#include <kmt.h>

#define SEM_TEST_MAX_BRACES 100

static thread_t t1, t2;
static sem_t empty;
static sem_t fill;
volatile static int counter;
volatile static char complete[2];

static void producer(void *ignore) {
    //int i = 0;
    while (counter < SEM_TEST_MAX_BRACES) {
        kmt->sem_wait(&empty);
        printf("(");
        
        //printf("%d", fill.count);
        kmt->sem_signal(&fill);
    }
    complete[0] = 1;
    while(1);
}

static void consumer(void *ignore) {
    //int i = 0;
    while (counter < SEM_TEST_MAX_BRACES) {
        kmt->sem_wait(&fill);
        printf(")");
        counter += 1;
        kmt->sem_signal(&empty);
    }
    complete[1] = 1;
    while(1);
}

void sem_test() {
    syslog("SEM_TEST", "Now performing semaphore test...");
    kmt->sem_init(&empty, "empty", 10);
    kmt->sem_init(&fill, "fill", 0);
    syslog("SEM_TEST", "Semaphores initialized");
    counter = 0;
    complete[0] = complete[1] = 0;
    
    syslog("SEM_TEST", " Creating consumer and producer threads...");
    kmt->create(&t1, consumer, NULL);
    kmt->create(&t2, producer, NULL);
    
    while (!(complete[0] && complete[1]));
    printf("\n");
    
    syslog("SEM_TEST", "Consumer and producer threads exited. Killing each thread");
    kmt->teardown(&t1);
    kmt->teardown(&t2);
    syslog("SEM_TEST", "All threads are killed");
}
