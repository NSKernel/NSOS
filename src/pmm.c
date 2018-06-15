#include <os.h>
#include <os/syslog.h>
#include <pmm.h>

void pmm_init();
void *pmm_alloc(size_t size);
void pmm_free(void *ptr);

MOD_DEF(pmm) {
    .init = pmm_init,
    .alloc = pmm_alloc,
    .free = pmm_free
};


void pmm_print_memory_layout() {
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

void pmm_init() {
    syslog("PMM", "Initializing PMM...");
    memory_head = (struct memory_block*)(_heap.start);
    memory_head->front = NULL;
    memory_head->inuse = 0;
    memory_head->next = (struct memory_block*)(_heap.end);
    kmt->spin_init(&kernel_memory_lock, "Kernel Memory Lock");
    syslog("PMM", "PMM initialization is done.");
}

void *pmm_alloc(size_t size) {
    size_t realallocsize = 1;
    size_t k = 0;
    void *startinblock;
    void *allocaddr;
    struct memory_block *current_front, *current_next;
    
    kmt->spin_lock(&kernel_memory_lock);
    
    struct memory_block *blockit = memory_head;
    
    while (realallocsize < size) {
        realallocsize <<= 1;
        k += 1;
    }
    
    startinblock = (void*)blockit + sizeof(struct memory_block);
    allocaddr = (void*)((((size_t)startinblock % realallocsize) == 0) ?
                (size_t)startinblock : 
                ((((size_t)startinblock >> k) << k) + realallocsize));
    while (blockit != NULL && blockit != _heap.end && (
           blockit->inuse == 1 ||  // block in use
           ((int)((size_t)blockit->next - (size_t)allocaddr - size) < 0)) // block not large enough
          ) {
        blockit = blockit->next;
        startinblock = (void*)blockit + sizeof(struct memory_block);
        allocaddr = (void*)((((size_t)startinblock % realallocsize) == 0) ?
                    (size_t)startinblock : 
                    ((((size_t)startinblock >> k) << k) + realallocsize));
    }
    
    if (blockit == NULL) { // no empty block that is large enough
        syslog("PMM", "No empty block that is large enough");
        pmm_print_memory_layout();
        kmt->spin_unlock(&kernel_memory_lock);
        return NULL;
    }
    
    if ((int)(allocaddr - startinblock - sizeof(struct memory_block)) > 0) { // enough room for block header
        ((struct memory_block*)(allocaddr - sizeof(struct memory_block)))->front = blockit;
        ((struct memory_block*)(allocaddr - sizeof(struct memory_block)))->inuse = 1;
        if ((int)((size_t)blockit->next - (size_t)allocaddr - size - sizeof(struct memory_block)) > 0) { // enough room for another block
            ((struct memory_block*)(allocaddr + size))->front = (struct memory_block*)(allocaddr - sizeof(struct memory_block));
            ((struct memory_block*)(allocaddr + size))->inuse = 0;
            ((struct memory_block*)(allocaddr + size))->next = blockit->next;
            ((struct memory_block*)(allocaddr - sizeof(struct memory_block)))->next = (struct memory_block*)(allocaddr + size);
            if (blockit->next != _heap.end) {
                blockit->next->front = (struct memory_block*)(allocaddr + size);
            }
            blockit->next = (struct memory_block*)(allocaddr - sizeof(struct memory_block));
            
            kmt->spin_unlock(&kernel_memory_lock);
            return (void*)allocaddr;
        }
        else {
            ((struct memory_block*)(allocaddr - sizeof(struct memory_block)))->next = blockit->next;
            if (blockit->next != _heap.end) {
                blockit->next->front = (struct memory_block*)(allocaddr - sizeof(struct memory_block));
            }
            blockit->next = (struct memory_block*)(allocaddr - sizeof(struct memory_block));
            
            kmt->spin_unlock(&kernel_memory_lock);
            return (void*)allocaddr;
        }
    }
    else {
        
        if ((int)((size_t)blockit->next - (size_t)allocaddr - size - sizeof(struct memory_block)) > 0) { // enough room for another block
            ((struct memory_block*)(allocaddr + size))->front = (struct memory_block*)(allocaddr - sizeof(struct memory_block));
            ((struct memory_block*)(allocaddr + size))->inuse = 0;
            ((struct memory_block*)(allocaddr + size))->next = blockit->next;
            
            if (blockit->next != _heap.end) {
                blockit->next->front = (struct memory_block*)(allocaddr + size);
            }
            blockit->next = (struct memory_block*)(allocaddr + size);
        }
        
        if (blockit->front == NULL) { // memory head
            memory_head = (struct memory_block*)(allocaddr - sizeof(struct memory_block));
            memory_head->next = blockit->next;
            memory_head->inuse = 1;
        }
        else {
            current_next = blockit->next;
            current_front = blockit->front;
            
            blockit->front->next = (struct memory_block*)(allocaddr - sizeof(struct memory_block));
            
            current_next->front = (struct memory_block*)(allocaddr - sizeof(struct memory_block));
            ((struct memory_block*)(allocaddr - sizeof(struct memory_block)))->next = current_next;
            ((struct memory_block*)(allocaddr - sizeof(struct memory_block)))->front = current_front;
            ((struct memory_block*)(allocaddr - sizeof(struct memory_block)))->inuse = 1;
        }
        kmt->spin_unlock(&kernel_memory_lock);
        return (void*)allocaddr;
    }
}

void pmm_free(void *ptr) {
    kmt->spin_lock(&kernel_memory_lock);
    
    struct memory_block *current = (struct memory_block*)(ptr - sizeof(struct memory_block));
    current->inuse = 0;
    while (current->front != NULL && current->front->inuse == 0) {
        current->front->next = current->next;
        current->next->front = current->front;
        current = current->front;
    }
    if (current->front == NULL) {
        memory_head = (struct memory_block *)_heap.start;
        memory_head->next = current->next;
        memory_head->front = NULL;
        memory_head->next->front = memory_head;
        memory_head->inuse = 0;
        current = memory_head;
    }
    while (current->next != _heap.end && current->next->inuse == 0) {
        current->next->front = current;
        current->next = current->next->next;
    }
    
    kmt->spin_unlock(&kernel_memory_lock);
}
