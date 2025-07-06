#pragma once

#include <Traits.hpp>

struct Memory {
    enum Role : unsigned char {
        APPLICATION,
        SYSTEM,
        COUNT,
    };

    typedef struct Page {
        struct Page *next;
    } Page;

    typedef struct Block {
        struct Block *next;
    } Block;

    typedef Block *Heap[Traits<Memory>::Page::ORDER + 1];

    static void init();
    static void *kmalloc();
    static void kfree(void *);
};

void *operator new(unsigned long, void *);
void *operator new(unsigned long, Memory::Role);
void operator delete(void *, unsigned long);
void operator delete(void *);
