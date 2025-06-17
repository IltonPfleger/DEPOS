#include <io/io.hpp>
#include <io/uart.hpp>
#include <memory.hpp>

typedef struct MemoryBlock {
    struct MemoryBlock *next;
} MemoryBlock;

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];
static MemoryBlock *available[Machine::Memory::ORDER + 1];
static uintptr_t BASE = (uintptr_t)__KERNEL_END__;

static unsigned int bytesToOrder(unsigned int bytes) {
    unsigned int order = 0;
    while ((1U << order) < bytes) order++;
    return order;
}

void Memory::init() {
    const unsigned int KERNEL_SIZE = __KERNEL_END__ - __KERNEL_START__;
    const unsigned int FREE =
        Machine::Memory::SIZE - KERNEL_SIZE - sizeof(MemoryBlock);
    for (unsigned int i = 0; i <= Machine::Memory::ORDER; i++)
        available[i] = nullptr;

    IO::out("Memory::init()\n");
    IO::out("KernelStart=%p | ", __KERNEL_START__);
    IO::out("KernelEnd=%p | ", __KERNEL_END__);
    IO::out("KernelSize=%dKB\n", KERNEL_SIZE / 1024);

    unsigned int remaining = FREE;
    uintptr_t current      = BASE;

    while (remaining) {
        unsigned int order = Machine::Memory::ORDER;
        while (order > 0 && (1U << order) > remaining) order--;
        unsigned int size  = (1U << order);
        MemoryBlock *block = reinterpret_cast<MemoryBlock *>(current);
        block->next        = available[order];
        available[order]   = block;
        remaining -= size;
        current += size;
        IO::out("MemoryBlock=%p | Order=%d\n", current, order);
    }

    IO::out("HeapStart=%p | ", BASE);
    IO::out("HeapEnd=%p | ", current);
    IO::out("HeapSize=%dMB\n", FREE / (1024 * 1024));
    IO::out("Memory::init(done)\n");
}

void *Memory::malloc(unsigned int bytes) {
    if (bytes >= Machine::Memory::SIZE) return 0;
    if (bytes == 0) return 0;

    unsigned int order           = bytesToOrder(bytes);
    unsigned int available_order = order;

    while (available_order <= Machine::Memory::ORDER &&
           !available[available_order])
        available_order++;

    while (available_order != order) {
        MemoryBlock *block         = available[available_order];
        available[available_order] = available[available_order]->next;
        available_order--;

        uintptr_t buddy = reinterpret_cast<uintptr_t>(block);
        buddy           = ((buddy - BASE) ^ (1 << available_order)) + BASE;
        block->next     = (MemoryBlock *)buddy;
        available[available_order] = block;
    }

    MemoryBlock *block         = available[available_order];
    available[available_order] = available[available_order]->next;
    IO::out("Memory::allocate(%d)[allocated=%d, return=%p]\n", bytes,
            (1U << order), block);
    return (void *)block;
}

void Memory::free(void *ptr) {
    unsigned int order = 0;
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    for (unsigned int i = 0; i <= Machine::Memory::ORDER; i++) {
        if ((addr - BASE) & (1UL << i)) order = i;
    }
    IO::out("%p\n", addr - BASE);
    return;
    // IO::out("Order: %d\n", order);
    //// if (!ptr || order >= Machine::Memory::ORDER) return;

    // while (order < Machine::Memory::ORDER) {
    //     uintptr_t buddy_addr = ((addr - BASE) ^ (1 << order)) + BASE;

    //    MemoryBlock *buddy    = 0;
    //    MemoryBlock **current = &available[order];
    //    while (current != 0 && *current != 0) {
    //        if (reinterpret_cast<uintptr_t>(*current) == buddy_addr) {
    //            buddy    = *current;
    //            *current = (*current)->next;
    //            break;
    //        }
    //        current = &((*current)->next);
    //    }

    //    if (buddy == 0) break;

    //    if (buddy_addr < addr) addr = buddy_addr;

    //    order++;
    //}
    // MemoryBlock *block = reinterpret_cast<MemoryBlock *>(addr);
    // block->next        = available[order];
    // available[order]   = block;
    // IO::out("Memory::free(%p)\n", ptr);
}
