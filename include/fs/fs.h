#ifndef _FS_H_
#define _FS_H_

#include <kernel.h>
#include <kmt.h>
#include <fs/dentry.h>
#include <simple_lock.h>

#define LOOKUP_DIRECTORY 0x00000001
#define LOOKUP_ACCESS    0x00000002

struct super_operations;
struct inode_operations;
struct file_operations;

struct file_system_type {
    const char *name;
    
    struct file_system_type *next;
    char *dev;
    struct dentry *(*mount)(struct file_system_type *fs, const char *source, void *data);
    struct super_operations *sop;
    struct dentry_operations *dop;
    struct inode_operations *iop;
    struct file_operations *fop;
};

struct super_operations {
    struct inode *(*alloc_inode)(struct file_system_type *fs);
    void (*destroy_inode)(struct inode *ip);
    void (*umount_begin)(struct file_system_type *fs);
};

struct vfsmount {
    int mnt_count;
    const char *mnt_devname;
    struct dentry *mnt_root;
    struct dentry *mnt_mountpoint;
    struct vfsmount *mnt_parent;
    struct vfsmount *mnt_next;
    struct file_system_type *mnt_fs;
};

struct inode {
    kuid_t i_uid;
    kgid_t i_gid;
    unsigned short i_acl;
    int i_ino;
    
    off_t i_size;
    sem_t i_rwsem;
    
    struct inode_operations *i_op;
    struct file_operations *i_fop;
};

struct inode_operations {
    struct dentry *(*lookup)(struct inode *ip, struct dentry *dp);
    int (*create)(struct inode *ip, struct dentry *dp);
    int (*mkdir)(struct inode *ip, struct dentry *dp);
    int (*rmdir)(struct inode *ip, struct dentry *dp);
    int (*rename)(struct inode *old_ip, struct dentry *old_dp, struct inode *new_ip, struct dentry *new_dp);
    void (*truncate)(struct inode *ip);
    void (*truncate_range)(struct inode *in, off_t start, off_t end);
};

struct file {
    spinlock_t f_lock;
    struct dentry *f_dentry;
    struct inode *f_inode;
    off_t f_pos;
    uint32_t f_mode;
    
    uint32_t f_count;
    
    struct file_operations *f_op;
};

struct file_operations {
    int (*open)(struct inode *ip, struct file *fp);
    off_t (*llseek)(struct file *fp, off_t offset, int whence);
    ssize_t (*read)(struct file *fp, char *buf, size_t count);
    ssize_t (*write)(struct file *fp, char *buf, size_t count);
    //int (*readdir)(struct file *fp, struct dentry *subdirs);
};

#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR   0x3
#define O_APPEND 0x10

#endif
