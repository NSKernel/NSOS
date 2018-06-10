#ifndef _DENTRY_H_
#define _DENTRY_H_

#include <kernel.h>
#include <fs/fs.h>
#include <simple_lock.h>

struct dentry_operations;

struct dentry {
    char *d_iname;
    struct inode *d_inode;
    struct dentry *d_parent;
    struct dentry *d_child;
    struct dentry *d_subdirs;
    int d_mounted;
    char d_isdir;
    
    struct dentry_operations *d_op;
};

struct dentry_operations {
    int (*d_revalidate)(struct dentry *dentry);
    int (*d_init)(struct dentry *dentry);
};

extern void d_drop(struct dentry *dp);
extern void d_dbgprint(struct dentry *dp);

#endif
