#include <os.h>
#include <os/syslog.h>

#include <kmt.h>
#include <procfs.h>
#include <devfs.h>

//#ifdef UNIT_TEST
#include <unittest.h>
//#endif

void launchd(void *x) {
    syslog("launchd", "launchd is now running");
    
    syslog("launchd", "mounting proc to /proc...");
    
    // mount profs
    vfs->mkdir("/proc", 0x0664); // no matter if it exists, we first *try* make the dir
    if (vfs->mount("/proc", procfs)) {
        syslog("launchd", "failed to mount proc");
        panic("Failed to mount proc");
    }
    
    syslog("launchd", "proc is mounted");
    
    syslog("launchd", "mounting udev to /dev...");
    
    // mount profs
    vfs->mkdir("/dev", 0x0666); // no matter if it exists, we first *try* make the dir
    if (vfs->mount("/dev", devfs)) {
        syslog("launchd", "failed to mount udev");
        panic("Failed to mount udev");
    }
    
    syslog("launchd", "udev is mounted");
    
    //#ifdef UNIT_TEST
    key_action = 0;
    syslog("launchd", "Press enter to start unit test");
    /*while (key_action == 0);
    pmm_test();
    key_action = 0;
    syslog("launchd", "PMM test is completed. Press enter to continue");
    while (key_action == 0);
    kmt_test();
    key_action = 0;
    syslog("launchd", "KMT test is completed. Press enter to continue");
    while (key_action == 0);
    sem_test();
    syslog("launchd", "SEM test is completed. Press enter to continue");
    */
    while (key_action == 0);
    vfs_test();
    
    syslog("launchd", "test complete");
    //#endif
    
    #ifndef UNIT_TEST
    syslog("launchd", "no task is found");
    #endif
    while (1);
}
