#ifndef _PROC_H_
#define _PROC_H_

#include <kernel.h>

extern struct dentry *proc_mount(struct file_system_type *fs, const char *source, void *data);

extern struct super_operations *proc_super_operations;
extern struct dentry_operations *proc_dentry_operations;
extern struct inode_operations *proc_inode_operations;
extern struct file_operations *proc_file_operations;
extern struct file_system_type *procfs;

#endif
