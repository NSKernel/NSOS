#include <os.h>
#include <vfs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <os/syslog.h>
#include <os/timer.h>

#define NULL_INO 0
#define RANDOM_INO 1
#define ZERO_INO 2



struct dentry *devfs_mount(struct file_system_type *fs, const char *source, void *data);

// super operations
struct inode *devfs_alloc_inode(struct file_system_type *fs);
void devfs_destroy_inode(struct inode *ip);

// dentry operations
int devfs_d_revalidate(struct dentry *dentry);
int devfs_d_init(struct dentry *dentry);

// inode operations
struct dentry *devfs_lookup(struct inode *ip, struct dentry *dp);
int devfs_create(struct inode *ip, struct dentry *dp);
int devfs_mkdir(struct inode *ip, struct dentry *dp);
int devfs_rmdir(struct inode *ip, struct dentry *dp);
void devfs_truncate(struct inode *ip);
void devfs_truncate_range(struct inode *ip, off_t start, off_t end);

// file operations
int devfs_open(struct inode *ip, struct file *fp);
off_t devfs_llseek(struct file *fp, off_t offset, int whence);
ssize_t devfs_read(struct file *fp, char *buf, size_t count);
ssize_t devfs_write(struct file *fp, char *buf, size_t count);

struct super_operations devfs_super_operations_obj = {
    .alloc_inode = devfs_alloc_inode,
    .destroy_inode = devfs_destroy_inode,
    .umount_begin = NULL
};
struct super_operations *devfs_super_operations = &devfs_super_operations_obj;

struct dentry_operations devfs_dentry_operations_obj = {
    .d_revalidate = devfs_d_revalidate,
    .d_init = devfs_d_init
};
struct dentry_operations *devfs_dentry_operations = &devfs_dentry_operations_obj;

struct inode_operations devfs_inode_operations_obj = {
    .lookup = devfs_lookup,
    .create = devfs_create,
    .mkdir = devfs_mkdir,
    .rmdir = devfs_rmdir,
    .truncate = devfs_truncate,
    .truncate_range = devfs_truncate_range
};
struct inode_operations *devfs_inode_operations = &devfs_inode_operations_obj;

struct file_operations devfs_file_operations_obj = {
    .open = devfs_open,
    .llseek = devfs_llseek,
    .read = devfs_read,
    .write = devfs_write
};
struct file_operations *devfs_file_operations = &devfs_file_operations_obj;

struct file_system_type devfs_obj = {
    .name = "devfs",
    .next = NULL,
    .dev = "udev",
    .mount = devfs_mount,
    .sop = &devfs_super_operations_obj,
    .dop = &devfs_dentry_operations_obj,
    .iop = &devfs_inode_operations_obj,
    .fop = &devfs_file_operations_obj
};
struct file_system_type *devfs = &devfs_obj;

struct inode *devfs_alloc_inode(struct file_system_type *fs) {
    struct inode *retinode = pmm->alloc(sizeof(struct inode));
    
    retinode->i_op = devfs_inode_operations;
    retinode->i_fop = devfs_file_operations; // NOT A FILE, HAVE NO FILE OPERATIONS
    // the devfs is unique and uses device udev, and in fact the inodes are not actually managed by the fs
    return retinode;
}

void devfs_destroy_inode(struct inode *ip) {
    pmm->free(ip);
}

//
int devfs_d_revalidate(struct dentry *dentry) {
    return 0; // always valid because it is a ram file system
}

int devfs_d_init(struct dentry *dentry) {
    dentry->d_op = devfs_dentry_operations;
    return 0;
}

struct dentry *devfs_lookup(struct inode *ip, struct dentry *dp) {
    if (dp->d_parent->d_parent == dp->d_parent) { // check if we are at root of the proc
        if (!strcmp(dp->d_iname, "null")) {
            ip->i_ino = NULL_INO;
            dp->d_inode = ip;
            dp->d_isdir = 0;
            return dp;
        }
        else if (!strcmp(dp->d_iname, "random")) {
            ip->i_ino = RANDOM_INO;
            ip->i_acl = ACL_OWNER_R | ACL_GROUP_R | ACL_OTHER_R;
            dp->d_inode = ip;
            dp->d_isdir = 0;
            return dp;
        }
        else if (!strcmp(dp->d_iname, "zero")) {
            ip->i_ino = ZERO_INO;
            ip->i_acl = ACL_OWNER_R | ACL_GROUP_R | ACL_OTHER_R;
            dp->d_inode = ip;
            dp->d_isdir = 0;
            return dp;
        }
    }
    return NULL;
}

int devfs_create(struct inode *ip, struct dentry *dp) {
    return -1;
}

int devfs_mkdir(struct inode *ip, struct dentry *dp) {
    return -1;
}

int devfs_rmdir(struct inode *ip, struct dentry *dp) {
    return -1;
}

// int kvfs_rename(struct inode *old_ip, struct dentry *old_dp, struct inode *new_ip, struct dentry *new_dp) not necessary

void devfs_truncate(struct inode *ip) {
    
}

void devfs_truncate_range(struct inode *ip, off_t start, off_t end) {
    
}


////////////////////////
//                    //
//  inode operations  //
//                    //
////////////////////////

int devfs_open(struct inode *ip, struct file *fp) {
    if (ip->i_ino != NULL_INO && ip->i_ino != RANDOM_INO && ip->i_ino != ZERO_INO) // pid non exists
        return -1;
    // "the driver is not required to declear a corresponding method"
    return 0;
}

off_t devfs_llseek(struct file *fp, off_t offset, int whence) {
    switch (whence) {
        case SEEK_SET:
            fp->f_pos = offset;
            return fp->f_pos;
        case SEEK_CUR:
            fp->f_pos += offset;
            return fp->f_pos;
        case SEEK_END:
            fp->f_pos = fp->f_inode->i_size + offset;
            return fp->f_pos;
        default:
            return -1;
    }
}

ssize_t devfs_read(struct file *fp, char *buf, size_t count) {
    int readcount = 0;
    switch (fp->f_dentry->d_inode->i_ino) {
      case NULL_INO:
        return 0;
      case RANDOM_INO:
        srand(getuptime32());
        while (readcount < count) {
            buf[readcount] = rand();
            readcount++;
        }
        return readcount;
      case ZERO_INO:
        while (readcount < count) {
            buf[readcount] = '\0';
            readcount++;
        }
        return readcount;
      default:
        return -1;
    }
}

// write is merely just an append operation
ssize_t devfs_write(struct file *fp, char *buf, size_t count) {
    switch (fp->f_dentry->d_inode->i_ino) {
      case NULL_INO:
        return count;
      case RANDOM_INO:
        return count;
      case ZERO_INO:
        return count;
    }
    return -1;
}

struct dentry *devfs_mount(struct file_system_type *fs, const char *source, void *data) {
    if (strlen(source) != strlen("udev") || strncmp(source, "udev", strlen("udev"))) {
        return NULL;
    }
    
    struct inode *ip = pmm->alloc(sizeof(struct inode));
    struct dentry *dentry = pmm->alloc(sizeof(struct dentry));
    
    ip->i_uid = 0;
    ip->i_gid = 0;
    ip->i_acl = 0x0666;
    ip->i_ino = -1;
    ip->i_op = devfs_inode_operations;
    ip->i_fop = NULL;
    
    
    dentry->d_iname = ""; // root has no name
    dentry->d_inode = ip;
    dentry->d_parent = dentry;
    dentry->d_subdirs = NULL;
    dentry->d_mounted = 0;
    dentry->d_isdir = 1;
    dentry->d_op = devfs_dentry_operations;
    
    fs->next = NULL;
    fs->dev = pmm->alloc(strlen(source) + 1);
    strncpy(fs->dev, source, strlen(source) + 1);
    
    return dentry;
}
