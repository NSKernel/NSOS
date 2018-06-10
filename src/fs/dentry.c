#include <os.h>
#include <vfs.h>
#include <fs/dentry.h>
#include <kmt.h>
#include <string.h>
#include <simple_lock.h>
#include <os/syslog.h>

void d_drop(struct dentry *dp) {
    struct dentry *denit;

    // remove the dentry from the chain
    if (dp == dp->d_parent->d_subdirs) {
        dp->d_parent->d_subdirs = dp->d_child;
    }
    else {
        denit = dp->d_parent->d_subdirs;
        while (denit->d_child != NULL && denit->d_child != dp) {
            denit = denit->d_child;
        }
        if (denit->d_child == NULL) {
            // failed to find the dentry. the dentry has already been droped.
        }
        else {
            denit->d_child = dp->d_child; // remove the dentry from the chain
        }
    }
}

void d_dbgprint(struct dentry *dp) {
    printf("struct dentry at 0x%08X\n", dp);
    printf("d_iname      %s\n", dp->d_iname);
    printf("d_inode      0x%08X\n", dp->d_inode);
    printf("d_parent     0x%08X\n", dp->d_parent);
    printf("d_child      0x%08X\n", dp->d_child);
    printf("d_subdirs    0x%08X\n", dp->d_subdirs);
    printf("d_mounted    %d\n", dp->d_mounted);
    printf("d_isdir      %d\n", dp->d_isdir);
    printf("d_op         0x%08X\n", dp->d_op);
}
