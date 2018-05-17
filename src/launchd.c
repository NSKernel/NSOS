#include <os.h>
#include <os/syslog.h>

#include <kmt.h>

//#ifdef UNIT_TEST
#include <unittest.h>
//#endif

void launchd(void *x) {
    syslog("launchd", "launchd is now running");
    
    //#ifdef UNIT_TEST
    key_action = 0;
    syslog("launchd", "Press enter to start unit test");
    while (key_action == 0);
    pmm_test();
    key_action = 0;
    syslog("launchd", "PMM test is completed. Press enter to continue");
    while (key_action == 0);
    kmt_test();
    key_action = 0;
    syslog("launchd", "KMT test is completed. Press enter to continue");
    while (key_action == 0);
    sem_test();
    
    syslog("launchd", "test complete");
    //#endif
    
    #ifndef UNIT_TEST
    syslog("launchd", "no task is found");
    #endif
    while (1);
}
