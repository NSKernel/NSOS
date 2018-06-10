#include <os.h>
#include <kmt.h>
#include <vfs.h>
#include <string.h>
#include <stdio.h>
#include <os/syslog.h>


#define KVFS_MAX_FILE 256

struct dentry *kvfs_mount(struct file_system_type *fs, const char *source, void *data);

// super operations
struct inode *kvfs_alloc_inode(struct file_system_type *fs);
void kvfs_destroy_inode(struct inode *ip);

// dentry operations
int kvfs_d_revalidate(struct dentry *dentry);
int kvfs_d_init(struct dentry *dentry);

// inode operations
struct dentry *kvfs_lookup(struct inode *ip, struct dentry *dp);
int kvfs_create(struct inode *ip, struct dentry *dp);
int kvfs_mkdir(struct inode *ip, struct dentry *dp);
int kvfs_rmdir(struct inode *ip, struct dentry *dp);
void kvfs_truncate(struct inode *ip);
void kvfs_truncate_range(struct inode *ip, off_t start, off_t end);

// file operations
int kvfs_open(struct inode *ip, struct file *fp);
off_t kvfs_llseek(struct file *fp, off_t offset, int whence);
ssize_t kvfs_read(struct file *fp, char *buf, size_t count);
ssize_t kvfs_write(struct file *fp, char *buf, size_t count);


struct kvfs_file_node {
    char inuse;
    void *start;
    void *end;
};

static struct kvfs_file_node kvfs_files[KVFS_MAX_FILE];

struct super_operations kvfs_super_operations_obj = {
    .alloc_inode = kvfs_alloc_inode,
    .destroy_inode = kvfs_destroy_inode
};
struct super_operations *kvfs_super_operations = &kvfs_super_operations_obj;

struct dentry_operations kvfs_dentry_operations_obj = {
    .d_revalidate = kvfs_d_revalidate,
    .d_init = kvfs_d_init
};
struct dentry_operations *kvfs_dentry_operations = &kvfs_dentry_operations_obj;

struct inode_operations kvfs_inode_operations_obj = {
    .lookup = kvfs_lookup,
    .create = kvfs_create,
    .mkdir = kvfs_mkdir,
    .rmdir = kvfs_rmdir,
    .truncate = kvfs_truncate,
    .truncate_range = kvfs_truncate_range
};
struct inode_operations *kvfs_inode_operations = &kvfs_inode_operations_obj;

struct file_operations kvfs_file_operations_obj = {
    .open = kvfs_open,
    .llseek = kvfs_llseek,
    .read = kvfs_read,
    .write = kvfs_write
};
struct file_operations *kvfs_file_operations = &kvfs_file_operations_obj;

struct file_system_type kvfs_obj = {
    .name = "kvfs",
    .next = NULL,
    .dev = "ramdisk0",
    .mount = kvfs_mount,
    .sop = &kvfs_super_operations_obj,
    .dop = &kvfs_dentry_operations_obj,
    .iop = &kvfs_inode_operations_obj,
    .fop = &kvfs_file_operations_obj
};
struct file_system_type *kvfs = &kvfs_obj;

struct inode *kvfs_alloc_inode(struct file_system_type *fs) {
    struct inode *retinode = pmm->alloc(sizeof(struct inode));
    
    retinode->i_op = kvfs_inode_operations;
    retinode->i_fop = kvfs_file_operations; // NOT A FILE, HAVE NO FILE OPERATIONS
    // the kvfs is unique and uses device ramdisk0, and in fact the inodes are not actually managed by the fs
    return retinode;
}

void kvfs_destroy_inode(struct inode *ip) {
    pmm->free(ip);
}

//
int kvfs_d_revalidate(struct dentry *dentry) {
    return 0; // always valid because it is a ram file system
}

int kvfs_d_init(struct dentry *dentry) {
    dentry->d_op = kvfs_dentry_operations;
    return 0;
}

struct dentry *kvfs_lookup(struct inode *ip, struct dentry *dp) {
    // shall just not happen to lookup this way
    return NULL;
}

int kvfs_create(struct inode *ip, struct dentry *dp) {
    int i;
    for (i = 0; i < KVFS_MAX_FILE && kvfs_files[i].inuse; i++);
    if (i >= KVFS_MAX_FILE) // no free inode
        return 1;
    
    kvfs_files[i].inuse = 1;
    kvfs_files[i].start = NULL;
    kvfs_files[i].end = NULL;
    
    ip->i_ino = i;
    ip->i_size = 0;
    kmt->sem_init(&(ip->i_rwsem), "inode semaphore", 1);
    ip->i_op = kvfs_inode_operations;
    ip->i_fop = kvfs_file_operations;
    
    dp->d_inode = ip;
    dp->d_isdir = 0;
    
    return 0;
}

int kvfs_mkdir(struct inode *ip, struct dentry *dp) {
    ip->i_ino = -1; // directory has no inode number
    // i_acl should be set by vfs
    ip->i_size = 0;
    
    kmt->sem_init(&(ip->i_rwsem), "inode semaphore", 1);
    ip->i_op = kvfs_inode_operations;
    ip->i_fop = NULL; // NOT A FILE, HAVE NO FILE OPERATIONS
    
    dp->d_inode = ip;
    dp->d_isdir = 1;
    return 0;
}

int kvfs_rmdir(struct inode *ip, struct dentry *dp) {
    return 0; // nothing will happen
}

// int kvfs_rename(struct inode *old_ip, struct dentry *old_dp, struct inode *new_ip, struct dentry *new_dp) not necessary

void kvfs_truncate(struct inode *ip) {
    if (kvfs_files[ip->i_ino].start != NULL) {
        pmm->free(kvfs_files[ip->i_ino].start);
    }
    ip->i_size = 0;
}

void kvfs_truncate_range(struct inode *ip, off_t start, off_t end) {
    char *newstart;
    
    if (start > end || end >= ip->i_size) {
        syslog("KVFS", "truncate range invalid");
        return;
    }
    
    ip->i_size -= (end - start);
    newstart = pmm->alloc(ip->i_size);
    memcpy(newstart, kvfs_files[ip->i_ino].start, start);
    memcpy(newstart + start, kvfs_files[ip->i_ino].start + end, (uint32_t)(kvfs_files[ip->i_ino].end - kvfs_files[ip->i_ino].start - end - 1));
    pmm->free(kvfs_files[ip->i_ino].start);
    kvfs_files[ip->i_ino].start = newstart;
    kvfs_files[ip->i_ino].end = newstart + ip->i_size;
}


////////////////////////
//                    //
//  inode operations  //
//                    //
////////////////////////

int kvfs_open(struct inode *ip, struct file *fp) {
    if (ip->i_ino == -1) // you cannot open a dir
        return 1;
    // "the driver is not required to declear a corresponding method"
    return 0;
}

off_t kvfs_llseek(struct file *fp, off_t offset, int whence) {
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

ssize_t kvfs_read(struct file *fp, char *buf, size_t count) {
    char *iterator = (char*)kvfs_files[fp->f_inode->i_ino].start + fp->f_pos;
    ssize_t readcount = 0;
    
    while ((int32_t)(kvfs_files[fp->f_inode->i_ino].end - (uint32_t)iterator) > 0 && readcount < count) {
        buf[readcount] = *iterator;
        iterator++;
        readcount++;
    }
    if (readcount < count) {
        buf[readcount] = '\n';
        readcount++;
    }
    
    return readcount;
}

// write is merely just an append operation
ssize_t kvfs_write(struct file *fp, char *buf, size_t count) {
    char *newstart = pmm->alloc(fp->f_inode->i_size + count);
    char *newend = newstart + fp->f_inode->i_size + count;
    
    if (kvfs_files[fp->f_inode->i_ino].start != NULL) { // copy the old stuff
        memcpy(newstart, kvfs_files[fp->f_inode->i_ino].start, fp->f_inode->i_size);
    }
    memcpy(newstart + fp->f_inode->i_size, buf, count);
    
    if (kvfs_files[fp->f_inode->i_ino].start != NULL) {
        pmm->free(kvfs_files[fp->f_inode->i_ino].start);
    }
    
    kvfs_files[fp->f_inode->i_ino].start = newstart;
    kvfs_files[fp->f_inode->i_ino].end = newend;
    
    fp->f_inode->i_size += count;
    
    return count;
}

struct dentry *kvfs_mount(struct file_system_type *fs, const char *source, void *data) {
    struct inode *ip = pmm->alloc(sizeof(struct inode));
    struct dentry *dentry = pmm->alloc(sizeof(struct dentry));
    int i;
    
    ip->i_uid = 0;
    ip->i_gid = 0;
    ip->i_acl = 0x777;
    ip->i_ino = -1;
    ip->i_op = kvfs_inode_operations;
    ip->i_fop = NULL;
    
    
    dentry->d_iname = ""; // root has no name
    dentry->d_inode = ip;
    dentry->d_parent = dentry;
    dentry->d_subdirs = NULL;
    dentry->d_mounted = 0; // though kvfs is mounted to root, we considered it as a special kind of mounting
    dentry->d_isdir = 1;
    dentry->d_op = kvfs_dentry_operations;
    
    fs->next = NULL;
    
    for (i = 0; i < KVFS_MAX_FILE; i++) {
        kvfs_files[i].inuse = 0;
        kvfs_files[i].start = NULL;
        kvfs_files[i].end = NULL;
    }
    
    return dentry;
}
