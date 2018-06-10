#ifndef _VFS_H_
#define _VFS_H_

#include <kernel.h>
#include <fs/fs.h>
#include <fs/dentry.h>
#include <fs/acl.h>
#include <simple_lock.h>

struct vfsmount root_mnt;
struct vfsmount *mnt_head;

struct dentry *root_dentry;

//struct inode root_inode;

struct nameidata {
    struct dentry *dentry;
    struct vfsmount *mnt;
};

struct fs_struct {
    struct dentry *root, *pwd, *altroot;
    struct vfsmount *rootmnt, *pwdmnt, *altrootmnt;
};

sem_t vfs_sem;

#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR   0x3

#endif
