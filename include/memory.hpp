#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <definitions.hpp>

struct Memory {
    typedef struct MemoryBlock {
        struct MemoryBlock* next;
    } MemoryBlock;

    static void init();
    static void* kmalloc();
    static void kfree(void*);

    // struct Heap {
    //     typedef struct HeapBlock {
    //         struct HeapBlock* next;
    //         uintptr_t size;
    //         bool free;
    //     } HeapBlock;

    //    HeapBlock* start;
    //    void* malloc(uintptr_t);
    //    void free(void*);
    //};
};

#endif
