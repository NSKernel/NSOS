#include <os.h>
#include <vfs.h>
#include <kvfs.h>
#include <kmt.h>
#include <os/syslog.h>
#include <stdio.h>
#include <string.h>
#include <unittest.h>

void do_tracefile(struct dentry *dentry) {
    struct vfsmount *mntit = mnt_head;

    while (dentry != root_dentry) {
        if (dentry->d_parent == dentry) { // this node needs to get to upper mount point
            mntit = mnt_head;
            while (mntit != NULL) {
                if (mntit->mnt_root == dentry) {
                    printf("m(0x%08X)", dentry);
                    dentry = mntit->mnt_mountpoint;
                    break;
                }
                mntit = mntit->mnt_next;
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

static thread_t t[4];
volatile static char thread_complete[4];

void threadfuncvfs(void *n) {
    while (1);
}

void threadfuncvfswrite(void *n) {
    int fd, i = 0;
    char testword[200];
    
    sprintf(testword, "Hello from thread %d\n", n);
    
    fd = vfs->open("/a/b/c.txt", O_RDWR | O_APPEND);
    
    syslog("KMT_TEST", "Thread %d started, opened fd is %d", (int)n, fd);
    tracefile(fd);
    
    while (i++ < 10) {
        vfs->write(fd, testword, strlen(testword));
    }
    
    vfs->close(fd);
    
    syslog("KMT_TEST", "Thread %d complete", (int)n);
    thread_complete[(int)n] = 1;
    while (1);
    
}

char vfs_test_buf[5000];

void vfs_test_single_thread_rw() {
    int fd1, fd2, fd3, readcount;
    char *testword1 = "Hello world!\nThis is a new file!\n";
    char *testword2 = "Hello world!\nThis is another new file in rootfs!\n";
    syslog("VFS_TEST", "Now performing signle thread file rw test...");
    
    syslog("VFS_TEST", "Creating new file at \"/a.txt\"");
    fd1 = vfs->open("/a.txt", O_RDWR);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd1);
    syslog("VFS_TEST", "Writing data \"Hello world!\\nThis is a new file!\\n\"to \"/a.txt\" with fd %d", fd1);
    vfs->write(fd1, testword1, strlen(testword1));
    syslog("VFS_TEST", "Reading out data from \"/a.txt\"");
    vfs->lseek(fd1, 0, SEEK_SET);
    readcount = vfs->read(fd1, vfs_test_buf, 150);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Writing data \"Hello world!\\nThis is a new file!\\n\"to \"/a.txt\" with fd %d from offset 1", fd1);
    vfs->lseek(fd1, 1, SEEK_SET);
    vfs->write(fd1, testword1, strlen(testword1));
    syslog("VFS_TEST", "Reading out data from \"/a.txt\"");
    vfs->lseek(fd1, 0, SEEK_SET);
    readcount = vfs->read(fd1, vfs_test_buf, 150);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Creating another file at \"/a/b/c.txt\"");
    fd2 = vfs->open("/a/b/c.txt", O_RDWR | O_APPEND);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd2);
    syslog("VFS_TEST", "Writing data \"Hello world!\\nThis is another new file in rootfs!\\n\"to \"/a/b/c.txt\" with fd %d", fd2);
    vfs->write(fd2, testword2, strlen(testword2));
    syslog("VFS_TEST", "Reading out data from \"/a/b/c.txt\"");
    vfs->lseek(fd2, 0, SEEK_SET);
    readcount = vfs->read(fd2, vfs_test_buf, 150);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Closing file \"/a.txt\"");
    vfs->close(fd1);
    
    syslog("VFS_TEST", "Reopening file \"/a.txt\"");
    fd1 = vfs->open("/a.txt", O_RDWR | O_APPEND);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd1);
    syslog("VFS_TEST", "Appending data \"Hello world!\\nThis is a new file!\\n\"to \"/a.txt\" to fd %d", fd1);
    vfs->write(fd1, testword1, strlen(testword1));
    syslog("VFS_TEST", "Reading out data from \"/a.txt\"");
    vfs->lseek(fd1, 0, SEEK_SET);
    readcount = vfs->read(fd1, vfs_test_buf, 150);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening path \"/a/b\" which shoud fail because b is a dir");
    fd3 = vfs->open("/a/b", O_RDWR | O_APPEND);
    syslog("VFS_TEST", "fd is %d", fd3);
    
    syslog("VFS_TEST", "Also creating file \"/a/b/c.txt/d.txt\" shoud fail because c.txt is a file");
    fd3 = vfs->open("/a/b/c.txt/d.txt", O_RDWR | O_APPEND);
    syslog("VFS_TEST", "fd is %d", fd3);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "But creating file \"/a/b.txt\" should work");
    fd3 = vfs->open("/a/b.txt", O_RDWR | O_APPEND);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd3);
    syslog("VFS_TEST", "fd is %d and it did work", fd3);
    syslog("VFS_TEST", "Writing data \"Hello world!\\nThis is a new file!\\n\"to \"/a/b.txt\" to fd %d", fd3);
    vfs->write(fd3, testword1, strlen(testword1));
    vfs->lseek(fd3, 0, SEEK_SET);
    syslog("VFS_TEST", "Reading out data from \"/a/b.txt\"");
    readcount = vfs->read(fd3, vfs_test_buf, 150);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    
    vfs->close(fd1);
    vfs->close(fd2);
    vfs->close(fd3);
    
    syslog("VFS_TEST", "Single thread file rw test is completed");
}

void vfs_test_mount() {
    int fd2, readcount;
    struct file_system_type *kvfsstruct;
    char *testword3 = "Hello world!\nThis is a file in a newly mounted fs!\n";
    
    syslog("VFS_TEST", "Now performing mount and unmount test");
    
    syslog("VFS_TEST", "Now trying to mount another KVFS to \"/a/b\"");
    kvfsstruct = pmm->alloc(sizeof(struct file_system_type));
    kvfsstruct->name = "kvfs";
    kvfsstruct->next = NULL;
    kvfsstruct->dev = "ramdisk1";
    kvfsstruct->mount = kvfs_mount;
    kvfsstruct->sop = kvfs_super_operations;
    kvfsstruct->dop = kvfs_dentry_operations;
    kvfsstruct->iop = kvfs_inode_operations;
    kvfsstruct->fop = kvfs_file_operations;
    if (vfs->mount("/a/b", kvfsstruct)) {
        syslog("VFS_TEST", "Mount failed!");
        panic("Test failed!");
    }
    syslog("VFS_TEST", "KVFS mounted to \"/a/b\"");
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Creating file at \"/a/b/c.txt\" in newly mounted fs");
    fd2 = vfs->open("/a/b/c.txt", O_RDWR | O_APPEND);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd2);
    syslog("VFS_TEST", "Writing data \"Hello world!\\nThis is a file in a newly mounted fs!!\\n\"to \"/a/b/c.txt\" with fd %d", fd2);
    vfs->write(fd2, testword3, strlen(testword3));
    vfs->lseek(fd2, 0, SEEK_SET);
    syslog("VFS_TEST", "Reading out data from \"/a/b/c.txt\"");
    readcount = vfs->read(fd2, vfs_test_buf, 150);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    syslog("VFS_TEST", "Closing file *\"/a/b/c.txt\"*");
    vfs->close(fd2);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Unmounting KVFS @ \"/a/b\"");
    if (vfs->unmount("/a/b")) {
        syslog("VFS_TEST", "Unmount failed!");
        panic("Test failed!");
    }
    syslog("VFS_TEST", "KVFS unmounted from \"/a/b\"");
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Reopening \"/a/b/c.txt\" in old fs");
    fd2 = vfs->open("/a/b/c.txt", O_RDWR | O_APPEND);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd2);
    syslog("VFS_TEST", "Reading out data from \"/a/b/c.txt\" and we should still get the old test words");
    readcount = vfs->read(fd2, vfs_test_buf, 150);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    
    vfs->close(fd2);
    
    
    syslog("VFS_TEST", "Mount-unmount test is completed");
}

void vfs_test_proc() {
    int fd1, fd2, fd3, readcount;
    
    syslog("VFS_TEST", "Now testing proc...");
    
    
    syslog("VFS_TEST", "Opening \"/proc/cpuinfo\"");
    fd1 = vfs->open("/proc/cpuinfo", O_RDONLY);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd1);
    syslog("VFS_TEST", "Reading out data from \"/proc/cpuinfo\"");
    readcount = vfs->read(fd1, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd1);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/proc/meminfo\"");
    fd2 = vfs->open("/proc/meminfo", O_RDONLY);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd2);
    syslog("VFS_TEST", "Reading out data from \"/proc/meminfo\"");
    readcount = vfs->read(fd2, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd2);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/proc/1\"");
    fd3 = vfs->open("/proc/1", O_RDONLY);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd3);
    syslog("VFS_TEST", "Reading out data from \"/proc/1\"");
    readcount = vfs->read(fd3, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd3);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    
    syslog("VFS_TEST", "Creating 4 threads...");
    int thread_count = 0;
    t[0].name = "test_thread_1";
    t[1].name = "test_thread_2";
    t[2].name = "test_thread_3";
    t[3].name = "test_thread_4";
    while (thread_count < 4) {
        kmt->create(&t[thread_count], threadfuncvfs, NULL);
        thread_count++;
    }
    syslog("VFS_TEST", "4 threads have been created");
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/proc/2\"");
    fd3 = vfs->open("/proc/2", O_RDONLY);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd3);
    syslog("VFS_TEST", "Reading out data from \"/proc/2\"");
    readcount = vfs->read(fd3, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd3);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/proc/3\"");
    fd3 = vfs->open("/proc/3", O_RDONLY);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd3);
    syslog("VFS_TEST", "Reading out data from \"/proc/3\"");
    readcount = vfs->read(fd3, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd3);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/proc/4\"");
    fd3 = vfs->open("/proc/4", O_RDONLY);
    syslog("VFS_TEST", "fd is %d", fd3);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd3);
    syslog("VFS_TEST", "Reading out data from \"/proc/4\"");
    readcount = vfs->read(fd3, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd3);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/proc/5\"");
    fd3 = vfs->open("/proc/5", O_RDONLY);
    syslog("VFS_TEST", "fd is %d", fd3);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd3);
    syslog("VFS_TEST", "Reading out data from \"/proc/5\"");
    readcount = vfs->read(fd3, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd3);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    
    syslog("VFS_TEST", "Killing all 4 threads...");
    
    
    kmt->teardown(&(t[0]));
    kmt->teardown(&(t[3]));
    kmt->teardown(&(t[1]));
    kmt->teardown(&(t[2]));
    
    
    syslog("VFS_TEST", "All 4 threads killed");
    
    syslog("VFS_TEST", "proc test is completed...");
}

void vfs_test_devfs() {
    int fd1, fd2, fd3, readcount;
    
    syslog("VFS_TEST", "Now testing devfs...");
    
    
    syslog("VFS_TEST", "Opening \"/dev/null\"");
    fd1 = vfs->open("/dev/null", O_RDONLY);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd1);
    syslog("VFS_TEST", "Reading out data from \"/dev/null\"");
    readcount = vfs->read(fd1, vfs_test_buf, 100);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/dev/zero\"");
    fd2 = vfs->open("/dev/zero", O_RDONLY);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd2);
    syslog("VFS_TEST", "Reading out 100 bytes from \"/dev/zero\"");
    readcount = vfs->read(fd2, vfs_test_buf, 100);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data in hex is: ", readcount);
    while (readcount--) {
        printf("%02X", vfs_test_buf[readcount]);
    }
    printf("\n");
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/dev/random\"");
    fd3 = vfs->open("/dev/random", O_RDONLY);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd3);
    syslog("VFS_TEST", "Reading out 100 bytes from \"/dev/random\"");
    readcount = vfs->read(fd3, vfs_test_buf, 100);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    while (readcount--) {
        printf("%01X", vfs_test_buf[readcount]);
    }
    printf("\n");
    
    
    vfs->close(fd1);
    vfs->close(fd2);
    vfs->close(fd3);
    
    syslog("VFS_TEST", "devfs test is completed...");
}

void vfs_test_multithread_one_file() {
    int fd, readcount = 0;
    syslog("VFS_TEST", "Now performing opening and writing one file from multiple threads...");
    
    syslog("VFS_TEST", "Opening \"/a/b/c.txt\"");
    fd = vfs->open("/a/b/c.txt", O_RDONLY);
    syslog("VFS_TEST", "fd is %d", fd);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd);
    syslog("VFS_TEST", "Reading out data from \"/a/b/c.txt\"");
    readcount = vfs->read(fd, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd);
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Creating 4 threads...");
    int thread_count = 0;
    t[0].name = "test_thread_1";
    t[1].name = "test_thread_2";
    t[2].name = "test_thread_3";
    t[3].name = "test_thread_4";
    while (thread_count < 4) {
        kmt->create(&t[thread_count], threadfuncvfswrite, (void *)thread_count);
        thread_count++;
    }
    syslog("VFS_TEST", "4 threads have been created to write to /a/b/c.txt");
    
    while (!(thread_complete[0] && thread_complete[1] && thread_complete[2] && thread_complete[3]));
    syslog("VFS_TEST", "All threads completed. Killing all threads");
    
    kmt->teardown(&(t[0]));
    kmt->teardown(&(t[3]));
    kmt->teardown(&(t[1]));
    kmt->teardown(&(t[2]));
    syslog("VFS_TEST", "All threads are killed");
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    syslog("VFS_TEST", "Opening \"/a/b/c.txt\"");
    fd = vfs->open("/a/b/c.txt", O_RDONLY);
    syslog("VFS_TEST", "fd is %d", fd);
    syslog("VFS_TEST", "Traced path of the file is");
    tracefile(fd);
    syslog("VFS_TEST", "Reading out data from \"/a/b/c.txt\"");
    readcount = vfs->read(fd, vfs_test_buf, 5000);
    vfs_test_buf[readcount] = '\0';
    syslog("VFS_TEST", "%d bytes read. Data is: ", readcount);
    printf("%s", vfs_test_buf);
    vfs->close(fd);
    
    syslog("VFS_TEST", "Multithread write test done");
}

void vfs_test() {
    syslog("VFS_TEST", "Now performing VFS test...");
    
    syslog("VFS_TEST", "Writing to STDOUT_FILENO");
    vfs->write(STDOUT_FILENO, "Hello world!\n", strlen("Hello world!\n"));
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    vfs_test_single_thread_rw();
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    vfs_test_mount();
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    vfs_test_proc();
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    vfs_test_devfs();
    
    syslog("VFS_TEST", "Press enter to continue\n");
    press_enter_to_continue();
    
    vfs_test_multithread_one_file();
    
    syslog("VFS_TEST", "VFS test is done");
}
