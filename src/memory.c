#include <definitions.h>
#include <io/io.h>

extern char __KERNEL_START__[];
extern char __KERNEL_END__[];

struct block_s {
    struct block_s* next;
};

static char* BASE;
static struct block_s* free[MEM_ORDER + 1];

void* memory_alloc(int order) {
    if (order > MEM_ORDER) return 0;

    uint32_t available_order = order;

    while (available_order <= MEM_ORDER && !memory.free[available_order]) available_order++;

    while (available_order != order) {
        struct block_s* block          = memory.free[available_order];
        memory.free[available_order--] = block->next;

        block->next                  = (void*)((((char*)block - memory.base) ^ (1 << available_order)) + memory.base);
        memory.free[available_order] = block;
    }

    struct block_s* block = memory.free[order];
    memory.free[order]    = block->next;
    return (void*)block;
}

void memory_free(void* ptr, uint32_t order) {
    if (!ptr || order > MEM_ORDER) return;
    char* block = (char*)ptr;

    while (order <= MEM_ORDER) {
        char* buddy = ((block - memory.base) ^ (1 << order)) + memory.base;

        struct block_s* current = memory.free[order];
        struct block_s* prev    = 0;

        while (current && (char*)current != buddy) {
            prev    = current;
            current = current->next;
        }

        if (!current) break;

        if (prev)
            prev->next = current->next;
        else
            memory.free[order] = current->next;

        if (buddy < block) block = buddy;
        order++;
    }
    (*(struct block_s*)(block)).next = memory.free[order];
    memory.free[order]               = (struct block_s*)block;
}

void memory_init() {
    const uint32_t KERNEL_SIZE = __KERNEL_END__ - __KERNEL_START__;
    const uint32_t FREE        = Machine::Memory::SIZE - KERNEL_SIZE - Machine::Memory::Page::SIZE;
    BASE                       = __KERNEL_END__;

    io_out("Memory::init()\n");
    io_out("KernelStart=%p | ", __KERNEL_START__);
    io_out("KernelEnd=%p | ", __KERNEL_END__);
    io_out("KernelSize=%dKB\n", KERNEL_SIZE / 1024);

    uint32_t remaining = FREE;
    char* current      = BASE;

    while (remaining) {
        uint32_t order = Machine::Memory::ORDER;
        while (order > 0 && (1U << order) > remaining) order--;
        uint32_t size         = (1 << order);
        struct block_s* block = (struct block_s)current;
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
//  for (int i = 0; i <= MEM_ORDER; i++) {
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
//    for (int i = 0; i <= MEM_ORDER; i++) {
//        struct block_s* current = memory.free[i];
//        while (current) {
//            kprint("%p ", current);
//            current = current->next;
//        }
//        kprint("%d \n", i);
//    }
//    kprint("\n");
//}
