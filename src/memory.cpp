#include <io/io.hpp>
#include <io/uart.hpp>
#include <memory.hpp>

extern "C" char __KERNEL_START__[];
extern "C" char __KERNEL_END__[];
static char *BASE = 0;

struct Memory::Block *Memory::available[Machine::Memory::ORDER + 1] = {0};

static unsigned int bytes_to_order(unsigned int bytes) {
    unsigned int order = 0;
    while ((1U << order) < bytes) order++;
    return order;
}

void Memory::init() {
    const unsigned int KERNEL_SIZE = __KERNEL_END__ - __KERNEL_START__;
    const unsigned int FREE        = Machine::Memory::SIZE - KERNEL_SIZE;
    BASE                           = __KERNEL_END__;

    IO::out("Memory::init()\n");
    IO::out("KernelStart=%p | ", __KERNEL_START__);
    IO::out("KernelEnd=%p | ", __KERNEL_END__);
    IO::out("KernelSize=%dKB\n", KERNEL_SIZE / 1024);

    unsigned int remaining = FREE;
    char *current          = BASE;

    while (remaining) {
        unsigned int order = Machine::Memory::ORDER;
        while (order > 0 && (1U << order) > remaining) order--;
        unsigned int size   = (1 << order);
        struct Block *block = reinterpret_cast<Block *>(current);
        block->next         = available[order];
        available[order]    = block;
        IO::out("Block=%p | Order=%d\n", current, order);
        remaining -= size;
        current += size;
    }

    IO::out("HeapStart=%p | ", BASE);
    IO::out("HeapEnd=%p | ", current);
    IO::out("HeapSize=%dMB\n", FREE / (1024 * 1024));
    IO::out("Memory::init(done)\n");
}

void *Memory::malloc(unsigned int bytes) {
    unsigned int order = bytes_to_order(bytes);
    if (order >= Machine::Memory::ORDER) return 0;

    unsigned int available_order = order;

    while (available_order <= Machine::Memory::ORDER && !available[available_order]) available_order++;

    while (available_order != order) {
        struct Block *block        = available[available_order];
        available[available_order] = available[available_order]->next;
        available_order--;

        char *buddy                = reinterpret_cast<char *>(block);
        buddy                      = ((buddy - BASE) ^ (1 << available_order)) + BASE;
        block->next                = (struct Block *)buddy;
        available[available_order] = block;
    }

    struct Block *block        = available[available_order];
    available[available_order] = available[available_order]->next;
    IO::out("Memory::allocate(return=%p)\n", block);
    return block;
}

void Memory::free(void *ptr, unsigned int bytes) {
    unsigned int order = bytes_to_order(bytes);
    if (!ptr || order >= Machine::Memory::ORDER) return;

    char *addr = reinterpret_cast<char *>(ptr);
    while (order < Machine::Memory::ORDER) {
        char *buddy_addr = ((addr - BASE) ^ (1 << order)) + BASE;

        struct Block *buddy    = 0;
        struct Block **current = &available[order];
        while (current != 0 && *current != 0) {
            if (reinterpret_cast<char *>(*current) == buddy_addr) {
                buddy    = *current;
                *current = (*current)->next;
                break;
            }
            current = &((*current)->next);
        }

        if (buddy == 0) break;

        if (buddy_addr < addr) addr = buddy_addr;

        order++;
    }
    struct Block *block = reinterpret_cast<struct Block *>(addr);
    block->next         = available[order];
    available[order]    = block;
}
