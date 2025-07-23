#pragma once

#include <Spin.hpp>
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

    typedef struct Heap {
        Block *blocks[Traits<Memory>::Page::ORDER + 1];
        Spin lock;
    } Heap;

    static void init();
    static void *kmalloc();
    static void kfree(void *);
    static inline Spin lock{};
};

void *operator new(unsigned long, void *);
void *operator new(unsigned long, Memory::Role);
void operator delete(void *, unsigned long);
void operator delete(void *);
