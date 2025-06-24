#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <definitions.hpp>

struct Memory {
    typedef struct Page {
        struct Page* next;
    } Page;

    static void init();
    static void* kmalloc();
    static void kfree(void*);

    struct Heap {
        typedef struct Block {
            struct Block* next;
            uintptr_t size;
            bool free;
        } Block;
        Block* start;
    };
};

void* operator new(unsigned long bytes, Memory::Heap& target);
void operator delete(void*, Memory::Heap& target);

#endif
