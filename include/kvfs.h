#ifndef _KVFS_H_
#define _KVFS_H_

#include <kernel.h>

extern struct dentry *kvfs_mount(struct file_system_type *fs, const char *source, void *data);

extern struct super_operations *kvfs_super_operations;
extern struct dentry_operations *kvfs_dentry_operations;
extern struct inode_operations *kvfs_inode_operations;
extern struct file_operations *kvfs_file_operations;
extern struct file_system_type *kvfs;

#endif
