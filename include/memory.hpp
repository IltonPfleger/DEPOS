#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <definitions.hpp>

struct Memory {
    struct Block {
        struct Block* next;
    };

    static struct Block* available[Machine::Memory::ORDER + 1];
    static void init();
    static void* alloc(uint32_t);
    static void free(void*, uint32_t);
};

#endif
