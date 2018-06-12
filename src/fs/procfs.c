#include <os.h>
#include <kmt.h>
#include <pmm.h>
#include <vfs.h>
#include <string.h>
#include <stdio.h>
#include <os/syslog.h>

struct dentry *proc_mount(struct file_system_type *fs, const char *source, void *data);

#define CPUINFO_INO -1
#define MEMINFO_INO -2

// super operations
struct inode *proc_alloc_inode(struct file_system_type *fs);
void proc_destroy_inode(struct inode *ip);

// dentry operations
int proc_d_revalidate(struct dentry *dentry);
int proc_d_init(struct dentry *dentry);

// inode operations
struct dentry *proc_lookup(struct inode *ip, struct dentry *dp);
int proc_create(struct inode *ip, struct dentry *dp);
int proc_mkdir(struct inode *ip, struct dentry *dp);
int proc_rmdir(struct inode *ip, struct dentry *dp);
void proc_truncate(struct inode *ip);
void proc_truncate_range(struct inode *ip, off_t start, off_t end);

// file operations
int proc_open(struct inode *ip, struct file *fp);
off_t proc_llseek(struct file *fp, off_t offset, int whence);
ssize_t proc_read(struct file *fp, char *buf, size_t count);
ssize_t proc_write(struct file *fp, char *buf, size_t count);

struct super_operations proc_super_operations_obj = {
    .alloc_inode = proc_alloc_inode,
    .destroy_inode = proc_destroy_inode,
    .umount_begin = NULL
};
struct super_operations *proc_super_operations = &proc_super_operations_obj;

struct dentry_operations proc_dentry_operations_obj = {
    .d_revalidate = proc_d_revalidate,
    .d_init = proc_d_init
};
struct dentry_operations *proc_dentry_operations = &proc_dentry_operations_obj;

struct inode_operations proc_inode_operations_obj = {
    .lookup = proc_lookup,
    .create = proc_create,
    .mkdir = proc_mkdir,
    .rmdir = proc_rmdir,
    .truncate = proc_truncate,
    .truncate_range = proc_truncate_range
};
struct inode_operations *proc_inode_operations = &proc_inode_operations_obj;

struct file_operations proc_file_operations_obj = {
    .open = proc_open,
    .llseek = proc_llseek,
    .read = proc_read,
    .write = proc_write
};
struct file_operations *proc_file_operations = &proc_file_operations_obj;

struct file_system_type procfs_obj = {
    .name = "proc",
    .next = NULL,
    .dev = "proc",
    .mount = proc_mount,
    .sop = &proc_super_operations_obj,
    .dop = &proc_dentry_operations_obj,
    .iop = &proc_inode_operations_obj,
    .fop = &proc_file_operations_obj
};
struct file_system_type *procfs = &procfs_obj;

struct inode *proc_alloc_inode(struct file_system_type *fs) {
    struct inode *retinode = pmm->alloc(sizeof(struct inode));
    
    retinode->i_op = proc_inode_operations;
    retinode->i_fop = proc_file_operations;
    // the procfs is unique and uses device proc, and in fact the inodes are not actually managed by the fs
    return retinode;
}

void proc_destroy_inode(struct inode *ip) {
    pmm->free(ip);
}

//
int proc_d_revalidate(struct dentry *dentry) {
    return 1; // always invalid, needs to lookup all the time
}

int proc_d_init(struct dentry *dentry) {
    dentry->d_op = proc_dentry_operations;
    return 0;
}

struct dentry *proc_lookup(struct inode *ip, struct dentry *dp) {
    kmt->spin_lock(&thread_lock);
    
    int pid = 0;
    char *pidstringit;
    // we have meminfo, cpuinfo, [pid]
    if (dp->d_parent->d_parent == dp->d_parent) { // check if we are at root of the proc
        if (!strcmp(dp->d_iname, "cpuinfo")) {
            ip->i_ino = CPUINFO_INO;
            dp->d_inode = ip;
            dp->d_isdir = 0;
            kmt->spin_unlock(&thread_lock);
            return dp;
        }
        else if (!strcmp(dp->d_iname, "meminfo")) {
            ip->i_ino = MEMINFO_INO;
            ip->i_acl = ACL_OWNER_R | ACL_GROUP_R | ACL_OTHER_R;
            dp->d_inode = ip;
            dp->d_isdir = 0;
            kmt->spin_unlock(&thread_lock);
            return dp;
        }
        else {
            pidstringit = dp->d_iname;
            if (pidstringit == NULL || *pidstringit == '\0') {
                kmt->spin_unlock(&thread_lock);
                return NULL;
            }
            if (*pidstringit == '0') { // pid starts from 1
                kmt->spin_unlock(&thread_lock);
                return NULL;
            }
            while (*pidstringit != '\0') {
                if (*pidstringit <= '9' && *pidstringit >= '0') {
                    pid = pid * 10 + *pidstringit - '0';
                    pidstringit++;
                }
                else {
                    kmt->spin_unlock(&thread_lock);
                    return NULL;
                }
            }
            if (pid <= KERNEL_MAX_THREAD && pid > 0 && thread_pool[pid] != NULL) {
                ip->i_ino = pid;
                ip->i_acl = ACL_OWNER_R | ACL_GROUP_R | ACL_OTHER_R;
                dp->d_inode = ip;
                dp->d_isdir = 0;
                kmt->spin_unlock(&thread_lock);
                return dp;
            }
            else {
                kmt->spin_unlock(&thread_lock);
                return NULL;
            }
        }
    }
    
    kmt->spin_unlock(&thread_lock);
    return NULL;
}

int proc_create(struct inode *ip, struct dentry *dp) {
    // read-only fs
    return -1;
}

int proc_mkdir(struct inode *ip, struct dentry *dp) {
    // read-only fs
    return -1;
}

int proc_rmdir(struct inode *ip, struct dentry *dp) {
    // read-only fs
    return -1;
}

// int proc_rename(struct inode *old_ip, struct dentry *old_dp, struct inode *new_ip, struct dentry *new_dp) not necessary

void proc_truncate(struct inode *ip) {
    // read-only fs
}

void proc_truncate_range(struct inode *ip, off_t start, off_t end) {
    // read-only fs
}


////////////////////////
//                    //
//  inode operations  //
//                    //
////////////////////////

int proc_open(struct inode *ip, struct file *fp) {
    if (ip->i_ino != CPUINFO_INO && ip->i_ino != MEMINFO_INO && thread_pool[ip->i_ino] == NULL) // pid non exists
        return -1;
    // "the driver is not required to declear a corresponding method"
    return 0;
}

off_t proc_llseek(struct file *fp, off_t offset, int whence) {
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


char filebuf[5000];

ssize_t proc_read(struct file *fp, char *buf, size_t count) {
    kmt->spin_lock(&thread_lock);
    
    int pid = fp->f_inode->i_ino;
    if (pid != CPUINFO_INO && pid != MEMINFO_INO && thread_pool[pid] == NULL) { // pid non exists
        kmt->spin_unlock(&thread_lock);
        return -1;
    }

    ssize_t readcount = 0;
    char *iterator = filebuf + fp->f_pos;
    struct memory_block *blockit = memory_head;
    uint32_t memcount = 0;
    
    filebuf[0] = '\0';
    if (pid == CPUINFO_INO) {
        sprintf(filebuf, "processor       : 0\n\
vendor_id       : Jiang Yanyan\n\
cpu family      : 0\n\
model           : 114514\n\
model name      : Shitty Abstract Machine\n\
stepping        : 1\n\
microcode       : 0x1\n\
cpu MHz         : 2333.333\n\
cache size      : 2333 KB\n\
physical id     : 0\n\
siblings        : 1\n\
core id         : 0\n\
cpu cores       : 1\n\
fpu             : no\n\
flags           : nothing-available\n\
bugs            : cpu_meltdown spectre_v1 spectre_v2 spec_store_bypass and *a lot*\n\
address sizes   : 20 bits physical, 32 bits virtual\n\
\n");
    }
    else if (pid == MEMINFO_INO) {
        sprintf(filebuf + strlen(filebuf), "\n           blockit    front      next       in use\n");
        while ((void*)(blockit) != _heap.end && memcount < 50) {
            sprintf(filebuf + strlen(filebuf), "block %3d: 0x%08X 0x%08X 0x%08X %s\n", memcount, blockit, blockit->front, blockit->next, (blockit->inuse ? "yes" : "no"));
            memcount += 1;
            blockit = blockit->next;
        }
    }
    else {
        sprintf(filebuf, "PID:   %d\nname:  %s\nuid:   %d\ngid:   %d\nsleep: %s\nstack: 0x%08X-0x%08X\n", pid, thread_pool[pid]->name, thread_pool[pid]->uid, thread_pool[pid]->gid, (thread_pool[pid]->sleep ? "true" : "false"), thread_pool[pid]->stack.start, thread_pool[pid]->stack.end);
    }
    
    while ((int32_t)((int32_t)filebuf + strlen(filebuf) - (int32_t)iterator) > 0 && readcount < count) {
        buf[readcount] = *iterator;
        iterator++;
        readcount++;
    }
    if (readcount < count) {
        buf[readcount] = '\n';
        readcount++;
    }
    
    kmt->spin_unlock(&thread_lock);
    return readcount;
}

ssize_t proc_write(struct file *fp, char *buf, size_t count) {
    // read-only fs
    return -1;
}

struct dentry *proc_mount(struct file_system_type *fs, const char *source, void *data) {
    if (strlen(source) != strlen("proc") || strncmp(source, "proc", strlen("proc"))) {
        return NULL;
    }
    
    struct inode *ip = pmm->alloc(sizeof(struct inode));
    struct dentry *dentry = pmm->alloc(sizeof(struct dentry));
    
    ip->i_uid = 0;
    ip->i_gid = 0;
    ip->i_acl = ACL_OWNER_R | ACL_GROUP_R | ACL_OTHER_R ;
    ip->i_ino = -3;
    ip->i_op = proc_inode_operations;
    ip->i_fop = NULL;
    
    
    dentry->d_iname = ""; // root has no name
    dentry->d_inode = ip;
    dentry->d_parent = dentry;
    dentry->d_subdirs = NULL;
    dentry->d_mounted = 0;
    dentry->d_isdir = 1;
    dentry->d_op = proc_dentry_operations;
    
    fs->next = NULL;
    fs->dev = pmm->alloc(strlen(source) + 1);
    strncpy(fs->dev, source, strlen(source) + 1);
    
    return dentry;
}
