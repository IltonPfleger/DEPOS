#ifndef MEMORY_H
#define MEMORY_H
#include <utils/definitions.hpp>

struct Memory {
    struct Block {
        struct Block* next;
    };

    static struct Block* free[Machine::Memory::ORDER + 1];
    static uintptr_t base;

    static void init(uintptr_t);
    static void* alloc(uint32_t);
    static void dalloc(void*, uint32_t);
    static uintptr_t convert_to_buddy_address(uintptr_t);
    static uintptr_t convert_from_buddy_address(uintptr_t);
};

#endif
