#include <os.h>
#include <vfs.h>
#include <kmt.h>
#include <os/syslog.h>
#include <stdio.h>
#include <string.h>

void do_tracefile(struct dentry *dentry) {
    struct vfsmount *mntit = mnt_head;

    while (dentry != root_dentry) {
        if (dentry->d_parent == dentry) { // this node needs to get to upper mount point
            mntit = mnt_head;
            while (mntit != NULL) {
                if (mntit->mnt_root == dentry) {
                    dentry = mntit->mnt_mountpoint;
                    break;
                }
            }
            if (mntit == NULL || dentry->d_mounted == 0) {
                syslog("VFS_TEST", "Mount issue! FS implementation has a bug!");
            }
        }
        
        printf("%s(0x%08X) <- ", dentry->d_iname, dentry);
        dentry = dentry->d_parent;
    }
    printf("/(0x%08X)\n", dentry);
}

void tracefile(int fd) {
    struct file *file = current->file_descriptors[fd];
    do_tracefile(file->f_dentry);
}

void vfs_test() {
    int fd1, fd2, fd3, readcount;
    char *testword1 = "Hello world!\nThis is a new file!\n";
    char *testword2 = "Hello world!\nThis is another new file!\n";
    char buf[200];
    syslog("VFS_TEST", "Now performing VFS test...");
    
    syslog("VFS_TEST", "Creating new file at \"/a.txt\"");
    fd1 = vfs->open("/a.txt", O_RDWR);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd1);
    syslog("VFS_TEST", "Writing data \"Hello world!\\nThis is a new file!\\n\"to \"/a.txt\" with fd %d", fd1);
    vfs->write(fd1, testword1, strlen(testword1));
    syslog("VFS_TEST", "Reading out data from \"/a.txt\"");
    readcount = vfs->read(fd1, buf, 150);
    buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", buf);
    
    syslog("VFS_TEST", "Creating another file at \"/a/b/c.txt\"");
    fd2 = vfs->open("/a/b/c.txt", O_RDWR);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd2);
    syslog("VFS_TEST", "Writing data \"Hello world!\\nThis is another new file!\\n\"to \"/a/b/c.txt\" with fd %d", fd2);
    vfs->write(fd2, testword2, strlen(testword2));
    syslog("VFS_TEST", "Reading out data from \"/a/b/c.txt\"");
    readcount = vfs->read(fd2, buf, 150);
    buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", buf);
    
    syslog("VFS_TEST", "Closing file \"/a.txt\"");
    vfs->close(fd1);
    
    syslog("VFS_TEST", "Reopening file \"/a.txt\"");
    fd1 = vfs->open("/a.txt", O_RDWR);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd1);
    syslog("VFS_TEST", "Appending data \"Hello world!\\nThis is a new file!\\n\"to \"/a.txt\" to fd %d", fd1);
    vfs->write(fd1, testword1, strlen(testword1));
    syslog("VFS_TEST", "Reading out data from \"/a.txt\"");
    readcount = vfs->read(fd1, buf, 150);
    buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", buf);
    
    syslog("VFS_TEST", "Creating another file \"/a/b\" which shoud fail because b is a dir");
    fd3 = vfs->open("/a/b", O_RDWR);
    syslog("VFS_TEST", "fd is %d", fd3);
    
    syslog("VFS_TEST", "Also creating file \"/a/b/c.txt/d.txt\" shoud fail because c.txt is a file");
    fd3 = vfs->open("/a/b/c.txt/d.txt", O_RDWR);
    syslog("VFS_TEST", "fd is %d", fd3);
    
    syslog("VFS_TEST", "But creating file \"/a/b.txt\" should work");
    fd3 = vfs->open("/a/b.txt", O_RDWR);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd3);
    syslog("VFS_TEST", "fd is %d and it did work", fd3);
    syslog("VFS_TEST", "Writing data \"Hello world!\\nThis is a new file!\\n\"to \"/a/b.txt\" to fd %d", fd3);
    vfs->write(fd3, testword1, strlen(testword1));
    syslog("VFS_TEST", "Reading out data from \"/a.txt\"");
    readcount = vfs->read(fd3, buf, 150);
    buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", buf);
    
    
    syslog("VFS_TEST", "VFS test is done");
}
