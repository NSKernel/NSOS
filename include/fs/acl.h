#ifndef _ACL_H_
#define _ACL_H_

#define ACL_OWNER_R 0x0400
#define ACL_OWNER_W 0x0200
#define ACL_OWNER_X 0x0100

#define ACL_GROUP_R 0x0040
#define ACL_GROUP_W 0x0020
#define ACL_GROUP_X 0x0010

#define ACL_OTHER_R 0x0004
#define ACL_OTHER_W 0x0002
#define ACL_OTHER_X 0x0001

#define ACL_DEFAULT 0x0664

int check_acl(kuid_t uid, kgid_t gid, struct inode *ip, int flags);

#endif
