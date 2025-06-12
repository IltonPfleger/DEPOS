#include <IO/IO.hpp>
#include <IO/UART.hpp>
#include <memory.hpp>

using UART0 = UART<0x10000000, 32000000, 115200>;

void Memory::init(uintptr_t base) {
    Memory::base = base;
    for (uint32_t i = 0; i < Machine::Memory::ORDER; i++) {
        IO<UART0>::print("%d\n", i);
        free[i] = 0;
    }
    free[Machine::Memory::ORDER]       = (struct Block*)base;
    free[Machine::Memory::ORDER]->next = 0;
}

void* Memory::alloc(uint32_t order) {
    if (order > Machine::Memory::ORDER) return 0;

    uint32_t available_order = order;

    while (available_order <= Machine::Memory::ORDER && !free[available_order]) available_order++;
    IO<UART0>::print("%d\n", available_order);

    // while (available_order != order) {
    //     IO<UART0>::print("%d\n",	available_order);
    //     struct Block* block = free[available_order];
    //     IO<UART0>::print("%d %p\n",	available_order, block->next);
    //     free[available_order--] = block->next;

    //    uintptr_t block_size  = (1 << available_order);
    //    uintptr_t block_ptr   = convert_from_buddy_address(convert_to_buddy_address((uintptr_t)block) ^ block_size);
    //    block->next           = (struct Block*)block_ptr;
    //    free[available_order] = block;
    //}

    // struct Block* block = free[order];
    // free[order]         = block->next;
    // return (void*)block;
}

void Memory::dalloc(void* ptr, uint32_t order) {
    if (!ptr || order > Machine::Memory::ORDER) return;
    uintptr_t block_ptr  = (uintptr_t)ptr;
    uintptr_t block_size = (1 << order);

    while (order <= Machine::Memory::ORDER) {
        uintptr_t buddy = convert_from_buddy_address(convert_to_buddy_address(block_ptr) ^ block_size);

        struct Block* current = free[order];
        struct Block* prev    = 0;

        while (current && (uintptr_t)current != buddy) {
            prev    = current;
            current = current->next;
        }

        if (!current) break;

        if (prev)
            prev->next = current->next;
        else
            free[order] = current->next;

        if (buddy < block_ptr) block_ptr = buddy;
        order++;
    }
    (*(struct Block*)(block_ptr)).next = free[order];
    free[order]                        = (struct Block*)block_ptr;
}

uintptr_t Memory::convert_to_buddy_address(uintptr_t ptr) { return ptr - base; }
uintptr_t Memory::convert_from_buddy_address(uintptr_t ptr) { return ptr + base; }

struct Memory::Block* Memory::free[Machine::Memory::ORDER + 1];
uintptr_t Memory::base;
