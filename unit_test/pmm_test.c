#include <os.h>
#include <os/syslog.h>
#include <stdio.h>
#include <pmm.h>
#include <kmt.h>

void print_memory_layout() {
    struct memory_block *blockit = memory_head;
    uint32_t count = 0;
    printf("\n           blockit    front      next       in use\n");
    while ((void*)(blockit) != _heap.end && count < 100) {
        printf("block %3d: 0x%08X 0x%08X 0x%08X %s\n", count, blockit, blockit->front, blockit->next, (blockit->inuse ? "yes" : "no"));
        count += 1;
        blockit = blockit->next;
    }
    printf("\n");
}

void pmm_test() {
    void *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10, *a11, *a12, *a13;
    syslog("PMM_TEST", "Now performing PMM test...");
    syslog("PMM_TEST", "The size of each block header is %d", sizeof(struct memory_block));
    
    print_memory_layout();
    syslog("PMM_TEST", "Random size allocating: allocating 13, 15, 7-byte spaces");
    a1 = pmm->alloc(13);
    a2 = pmm->alloc(15);
    a3 = pmm->alloc(7);
    print_memory_layout();
    syslog("PMM_TEST", "Freeing all memory");
    pmm->free(a1);
    pmm->free(a2);
    pmm->free(a3);
    print_memory_layout();
    syslog("PMM_TEST", "Allocating 4 16-byte spaces");
    a1 = pmm->alloc(16);
    print_memory_layout();
    a2 = pmm->alloc(16);
    print_memory_layout();
    a3 = pmm->alloc(16);
    print_memory_layout();
    a13 = pmm->alloc(16);
    print_memory_layout();
    syslog("PMM_TEST", "Freeing a1: 0x%08X", a1);
    pmm->free(a1);
    print_memory_layout();
    syslog("PMM_TEST", "Freeing a3: 0x%08X", a3);
    pmm->free(a3);
    print_memory_layout();
    syslog("PMM_TEST", "Freeing a2: 0x%08X", a2);
    pmm->free(a2);
    print_memory_layout();
    //syslog("PMM_TEST", "Freeing a4: 0x%08X", a4);
    //pmm->free(a4);
    print_memory_layout();
    syslog("PMM_TEST", "Allocating 8 32-byte spaces");
    a1 = pmm->alloc(32);
    //print_memory_layout();
    a5 = pmm->alloc(32);
    //print_memory_layout();
    a2 = pmm->alloc(32);
    a6 = pmm->alloc(32);
    a3 = pmm->alloc(32);
    a7 = pmm->alloc(32);
    a4 = pmm->alloc(32);
    a8 = pmm->alloc(32);
    print_memory_layout();
    syslog("PMM_TEST", "Freeing 1st, 3rd, 5th, 7th 32-byte spaces");
    pmm->free(a1);
    pmm->free(a2);
    pmm->free(a3);
    pmm->free(a4);
    print_memory_layout();
    syslog("PMM_TEST", "Allocating 4 8-byte and 16-byte spaces");
    a1 = pmm->alloc(8);
    a2 = pmm->alloc(16);
    a3 = pmm->alloc(8);
    a4 = pmm->alloc(16);
    a9 = pmm->alloc(8);
    a10 = pmm->alloc(16);
    a11 = pmm->alloc(8);
    a12 = pmm->alloc(16);
    print_memory_layout();
    syslog("PMM_TEST", "Freeing all memory");
    pmm->free(a1);
    pmm->free(a2);
    pmm->free(a3);
    pmm->free(a4);
    pmm->free(a5);
    pmm->free(a6);
    pmm->free(a7);
    pmm->free(a8);
    pmm->free(a9);
    pmm->free(a10);
    pmm->free(a11);
    pmm->free(a12);
    pmm->free(a13);
    print_memory_layout();
    syslog("PMM_TEST", "PMM test is done");
}
