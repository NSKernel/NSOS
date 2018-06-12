#ifndef _DEVFS_H_
#define _DEVFS_H_

#include <kernel.h>

extern struct dentry *devfs_mount(struct file_system_type *fs, const char *source, void *data);

extern struct super_operations *devfs_super_operations;
extern struct dentry_operations *devfs_dentry_operations;
extern struct inode_operations *devfs_inode_operations;
extern struct file_operations *devfs_file_operations;
extern struct file_system_type *devfs;

#endif
