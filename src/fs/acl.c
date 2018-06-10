#include <os.h>
#include <kmt.h>
#include <vfs.h>
#include <path.h>
#include <string.h>
#include <simple_lock.h>
#include <os/syslog.h>

int check_acl(kuid_t uid, kgid_t gid, struct inode *ip, int flags) {
    if (flags & R_OK) {
        if (uid == ip->i_uid) {
            if (!(ip->i_acl & ACL_OWNER_R)) {
                return 1;
            }
        }
        else if (gid == ip->i_gid) {
            if (!(ip->i_acl & ACL_GROUP_R)) {
                return 1;
            }
        }
        else {
            if (!(ip->i_acl & ACL_OTHER_R)) {
                return 1;
            }
        }
    }
    
    if (flags & W_OK) {
        if (uid == ip->i_uid) {
            if (!(ip->i_acl & ACL_OWNER_W)) {
                return 1;
            }
        }
        else if (gid == ip->i_gid) {
            if (!(ip->i_acl & ACL_GROUP_W)) {
                return 1;
            }
        }
        else {
            if (!(ip->i_acl & ACL_OTHER_W)) {
                return 1;
            }
        }
    }
    return 0;
}
