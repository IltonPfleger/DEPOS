#ifndef MEMORY_H
#define MEMORY_H
#include <utils/definitions.h>

struct memory_block_s {
    struct memory_block_s* next;
};

struct {
    uintptr_t base;
    struct memory_block_s* free[MEM_ORDER + 1];
} memory;

void* alloc(uint32_t order)
{
    if (order > MEM_ORDER) return 0;

    uint32_t available_order = order;

    while (available_order <= MEM_ORDER && !memory.free[available_order]) available_order++;

    while (available_order != order) {
        struct memory_block_s* block   = memory.free[available_order];
        memory.free[available_order--] = block->next;

        block->next = (void*)((((uintptr_t)block - memory.base) ^ (1 << available_order)) + memory.base);
        memory.free[available_order] = block;
    }

    struct memory_block_s* block = memory.free[order];
    memory.free[order]           = block->next;
    return (void*)block;
}

void dalloc(void* ptr, uint32_t order)
{
    if (!ptr || order > MEM_ORDER) return;
    uintptr_t block = (uintptr_t)ptr;

    while (order <= MEM_ORDER) {
        uintptr_t buddy = ((block - memory.base) ^ (1 << order)) + memory.base;

        struct memory_block_s* current = memory.free[order];
        struct memory_block_s* prev    = 0;

        while (current && (uintptr_t)current != buddy) {
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
    (*(struct memory_block_s*)(block)).next = memory.free[order];
    memory.free[order]                      = (struct memory_block_s*)block;
}

void memory_init()
{
    extern const uint8_t __KERNEL_END__[];
    memory.base = (((uintptr_t)__KERNEL_END__ + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

    for (int i = 0; i <= MEM_ORDER; i++) memory.free[i] = 0;

    memory.free[MEM_ORDER]       = (struct memory_block_s*)memory.base;
    memory.free[MEM_ORDER]->next = 0;
}

#endif

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
//        struct memory_block_s* current = memory.free[i];
//        while (current) {
//            kprint("%p ", current);
//            current = current->next;
//        }
//        kprint("%d \n", i);
//    }
//    kprint("\n");
//}
