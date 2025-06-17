#include <definitions.h>
#include <io/io.h>

extern char __KERNEL_START__[];
extern char __KERNEL_END__[];

struct block_s {
    struct block_s* next;
};

static char* BASE;
static struct block_s* available[machine.memory.order + 1];

void* memory_alloc(unsigned int order) {
    if (order > machine.memory.order) return 0;

    unsigned int available_order = order;

    while (available_order <= machine.memory.order && !available[available_order]) available_order++;

    while (available_order != order) {
        struct block_s* block          = available[available_order];
        available[available_order--] = block->next;

        block->next                  = (void*)((((char*)block - BASE) ^ (1 << available_order)) + BASE);
        available[available_order] = block;
    }

    struct block_s* block = available[order];
    available[order]    = block->next;
    return (void*)block;
}

void memory_free(void* ptr, unsigned int order) {
    if (!ptr || order > machine.memory.order) return;
    char* block = (char*)ptr;

    while (order <= machine.memory.order) {
        char* buddy = ((block - BASE) ^ (1 << order)) + BASE;

        struct block_s* current = available[order];
        struct block_s* prev    = 0;

        while (current && (char*)current != buddy) {
            prev    = current;
            current = current->next;
        }

        if (!current) break;

        if (prev)
            prev->next = current->next;
        else
            available[order] = current->next;

        if (buddy < block) block = buddy;
        order++;
    }
    (*(struct block_s*)(block)).next = available[order];
    available[order]               = (struct block_s*)block;
}

void memory_init() {
    const unsigned int KERNEL_SIZE = __KERNEL_END__ - __KERNEL_START__;
    const unsigned int FREE        = machine.memory.size - KERNEL_SIZE - machine.memory.page.size;
    BASE                       = __KERNEL_END__;

    io_out("Memory::init()\n");
    io_out("KernelStart=%p | ", __KERNEL_START__);
    io_out("KernelEnd=%p | ", __KERNEL_END__);
    io_out("KernelSize=%dKB\n", KERNEL_SIZE / 1024);

    unsigned int remaining = FREE;
    char* current      = BASE;

    while (remaining) {
        unsigned int order = machine.memory.order;
        while (order > 0 && (1U << order) > remaining) order--;
        unsigned int size         = (1 << order);
        struct block_s* block = (struct block_s*)current;
        block->next           = available[order];
        available[order]      = block;
        io_out("block_s=%p | Order=%d\n", current, order);
        remaining -= size;
        current += size;
    }

    io_out("HeapStart=%p | ", BASE);
    io_out("HeapEnd=%p | ", current);
    io_out("HeapSize=%dMB\n", FREE / (1024 * 1024));
    io_out("Memory::init(done)\n");
}

// #include <utils/kprint.h>
//  for (int i = 0; i <= machine.memory.order; i++) {
//		//kprint("%d ", i);
//          void* temp  = alloc(i);
//          void* temp2 = alloc(i);
//          if (!temp || !temp2) {
//              kprint("Cannot Allocate: %d\n", i);
//          }
//          //print_free();
//          dalloc(temp, i);
//          dalloc(temp2, i);
//          //print_free();
//      }
//
//      kprint("Done!\n\n");
//
////void print_free()
//{
//    for (int i = 0; i <= machine.memory.order; i++) {
//        struct block_s* current = available[i];
//        while (current) {
//            kprint("%p ", current);
//            current = current->next;
//        }
//        kprint("%d \n", i);
//    }
//    kprint("\n");
//}
