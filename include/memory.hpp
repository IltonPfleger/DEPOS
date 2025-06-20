#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <definitions.hpp>

struct Memory {
    typedef struct PBlock {
        struct PBlock* next;
    } PBlock;

    static void init();
    static void* kmalloc();
    static void kfree(void*);

    struct Heap {
        typedef struct HBlock {
            struct HBlock* next;
            uintptr_t size;
            bool free;
        } HBlock;
        HBlock* start;
    };
};

void* operator new(unsigned long bytes, Memory::Heap& target);
void operator delete(void*, Memory::Heap& target);

#endif
