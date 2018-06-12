#include <os.h>
#include <kmt.h>
#include <vfs.h>
#include <path.h>
#include <kvfs.h>
#include <procfs.h>
#include <string.h>
#include <simple_lock.h>
#include <os/syslog.h>

void vfs_init();
int vfs_access(const char *path, int mode);
int vfs_mount(const char *path, filesystem_t *fs);
int vfs_unmount(const char *path);
int vfs_open(const char *path, int flags);
ssize_t vfs_read(int fd, void *buf, size_t nbyte);
ssize_t vfs_write(int fd, void *buf, size_t nbyte);
off_t vfs_lseek(int fd, off_t offset, int whence);
int vfs_close(int fd);
void vfs_register_filesystem(filesystem_t *fs);
int vfs_mkdir(const char *path, unsigned short mode);

MOD_DEF(vfs) {
    .init = vfs_init,
    .access = vfs_access,
    .mount = vfs_mount,
    .unmount = vfs_unmount,
    .open = vfs_open,
    .read = vfs_read,
    .write = vfs_write,
    .lseek = vfs_lseek,
    .close = vfs_close,
    .mkdir = vfs_mkdir
};

void vfs_init() {
    syslog("VFS", "Initializing VFS..."); 
    
    kmt->sem_init(&(vfs_sem), "vfs semaphore", 1);
    
    // manually build the root directory and mount the kvfs as the root fs
    syslog("VFS", "Mounting kvfs to root...");
    
    root_dentry = kvfs->mount(kvfs, "ramdisk0", "");
    
    if (root_dentry == NULL) {
        syslog("VFS", "Failed to mount kvfs to root. Root fs failed to build itself.");
        panic("Root fs failed to mount.");
    }
    
    root_mnt.mnt_count = 1;
    root_mnt.mnt_devname = "ramdisk0";
    root_mnt.mnt_root = root_dentry;
    root_mnt.mnt_mountpoint = root_dentry;
    root_mnt.mnt_parent = NULL;
    root_mnt.mnt_next = NULL;
    root_mnt.mnt_fs = kvfs;
    
    mnt_head = &root_mnt;
    
    syslog("VFS", "kvfs is mounted.");
    syslog("VFS", "VFS initialization is done.");
}

int vfs_access(const char *path, int mode) {
    kmt->sem_wait(&vfs_sem);
    
    struct nameidata nd;
    struct dentry *retden;
    
    retden = path_lookup(path, 0, &nd);
    
    if (retden == NULL) {
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    if (mode == F_OK) {
        kmt->sem_signal(&vfs_sem);
        return 0;
    }
    
    if (mode & R_OK) {
        if (check_acl(current->uid, current->gid, retden->d_inode, R_OK)) {
            kmt->sem_signal(&vfs_sem);
            return -1;
        }
    }
    
    if (mode & W_OK) {
        if (check_acl(current->uid, current->gid, retden->d_inode, W_OK)) {
            kmt->sem_signal(&vfs_sem);
            return -1;
        }
    }
    
    kmt->sem_signal(&vfs_sem);
    return 0;
}

int vfs_mount(const char *path, filesystem_t *fs) {
    kmt->sem_wait(&vfs_sem);
    
    struct nameidata nd;
    struct dentry *retden;
    struct dentry *mount_root;
    struct vfsmount *mntfs;
    
    if (current->uid != 0 && current->gid != 0) { // if not root or not in sudoers
        syslog("VFS", "failed to mount %s: only root can do that", fs->name);
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    retden = path_lookup(path, 0, &nd);
    
    if (retden == NULL) { // if path is not found
        syslog("VFS", "failed to mount %s: directory not exists");
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    if (retden->d_isdir == 0) { // if path is not a directory
        syslog("VFS", "failed to mount %s: not a directory");
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    syslog("VFS", "mounting %s to %s...", fs->name, path);
    
    if (fs->mount == NULL) {
        syslog("VFS", "failed to mount %s: this fs doesn't support mounting behaviour");
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    mount_root = fs->mount(fs, fs->dev, "");
    if (mount_root == NULL) {
        syslog("VFS", "failed to mount %s: mount() failed");
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    mntfs = pmm->alloc(sizeof(struct vfsmount));
    mntfs->mnt_count = 1;
    mntfs->mnt_root = mount_root;
    mntfs->mnt_mountpoint = retden;
    mntfs->mnt_parent = nd.mnt;
    mntfs->mnt_next = mnt_head;
    mntfs->mnt_fs = fs;
    mnt_head = mntfs;
    retden->d_mounted += 1;
    
    syslog("VFS", "%s mounted.", fs->name);
    
    kmt->sem_signal(&vfs_sem);
    return 0;
}


int vfs_unmount(const char *path) {
    kmt->sem_wait(&vfs_sem);
    
    struct nameidata nd;
    struct dentry *retden;
    struct vfsmount *mntit;
    struct vfsmount *mntremain;
    
    if (current->uid != 0 && current->gid != 0) { // if not root or not in sudoers
        syslog("VFS", "failed to unmount %s: only root can do that", path);
        kmt->sem_signal(&vfs_sem);
        return 1;
    }
    
    retden = path_lookup(path, 0, &nd);
    
    if (retden == NULL) { // if path is not found
        syslog("VFS", "failed to unmount %s: directory not exists");
        kmt->sem_signal(&vfs_sem);
        return 1;
    }
    
    mntit = mnt_head;
    while (mntit != NULL && mntit->mnt_root != retden) { // pick the corresponding mounted fs
        mntit = mntit->mnt_next;
    }
    if (mntit == NULL) {
        syslog("VFS", "failed to unmount %s: path is not a mountpoint.", path);
        kmt->sem_signal(&vfs_sem);
        return 1;
    }
    
    mntit->mnt_mountpoint->d_mounted -= 1;
    // drop this mount
    if (mntit == mnt_head) 
        mnt_head = mntit->mnt_next;
    else {
        mntremain = mnt_head;
        while (mntremain->mnt_next != mntit)
            mntremain = mntremain->mnt_next;
        mntremain->mnt_next = mntit->mnt_next;
    }
    if (mntit->mnt_fs->sop->umount_begin != NULL)
        mntit->mnt_fs->sop->umount_begin(mntit->mnt_fs);
    pmm->free(mntit->mnt_fs);
    pmm->free(mntit);
    
    kmt->sem_signal(&vfs_sem);
    return 0;
}


int vfs_open(const char *path, int flags) {
    struct nameidata nd;
    struct dentry *retden;
    struct file *newfile;
    int i;
    
    kmt->sem_wait(&vfs_sem);

    retden = make_path(path, 0, &nd, ACL_DEFAULT);
    
    if (retden == NULL) {
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    newfile = pmm->alloc(sizeof(struct file));
    kmt->spin_init(&(newfile->f_lock), "file lock");
    newfile->f_dentry = retden;
    newfile->f_inode = retden->d_inode;
    newfile->f_pos = 0;
    newfile->f_count = 0;
    newfile->f_op = newfile->f_inode->i_fop;
    
    for (i = 0; i < MAX_FILE_PER_THREAD; i++) {
        if (current->file_descriptors[i] == NULL) {
            break;
        }
    }
    // no free file descriptors
    if (i == MAX_FILE_PER_THREAD) {
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    current->file_descriptors[i] = newfile;
    
    kmt->sem_signal(&vfs_sem);
    return i;
}

ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
    kmt->sem_wait(&vfs_sem);
    
    ssize_t readcount = 0;
    if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) // we currently don't support stdin
        readcount = current->file_descriptors[fd]->f_op->read(current->file_descriptors[fd], buf, nbyte);
    
    kmt->sem_signal(&vfs_sem);
    return readcount;
}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
    kmt->sem_wait(&vfs_sem);
    
    ssize_t writecount = 0;
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        while (nbyte-- != 0) {
            printf("%c", *(char *)(buf++));
            writecount++;
        }
    }
    else if (fd != STDIN_FILENO) { // we currently don't support stdin
        writecount = current->file_descriptors[fd]->f_op->write(current->file_descriptors[fd], buf, nbyte);
    }
    
    kmt->sem_signal(&vfs_sem);
    return writecount;
}

off_t vfs_lseek(int fd, off_t offset, int whence) {
    kmt->sem_wait(&vfs_sem);
    
    ssize_t pos = 0;
    if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) // we currently don't support stdin
        pos = current->file_descriptors[fd]->f_op->llseek(current->file_descriptors[fd], offset, whence);
    
    kmt->sem_signal(&vfs_sem);
    return pos;
}

int vfs_close(int fd) {
    if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) { // we currently don't support stdin
        if (current->file_descriptors[fd] != NULL) {
            pmm->free(current->file_descriptors[fd]);
            current->file_descriptors[fd] = NULL;
            return 0;
        }
    }
    return -1;
}


int vfs_mkdir(const char *path, unsigned short mode) {
    kmt->sem_wait(&vfs_sem);
    
    struct nameidata nd;
    struct dentry *retden;
    
    retden = path_lookup(path, 0, &nd);
    
    if (retden != NULL) { // path exists
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    retden = make_path(path, LOOKUP_DIRECTORY, &nd, mode);
    
    if (retden == NULL) { // failed to create path
        kmt->sem_signal(&vfs_sem);
        return -1;
    }
    
    kmt->sem_signal(&vfs_sem);
    return 0;
}
