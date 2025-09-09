#pragma once

#include <Lists.hpp>
#include <Spin.hpp>
#include <Traits.hpp>

struct Memory {
    enum Role : unsigned char {
        APPLICATION,
        SYSTEM,
        COUNT,
    };

    // typedef struct Page {
    //     struct Page *next;
    // } Page;

    typedef struct Block {
        struct Block *next;
    } Block;

    typedef struct Heap {
        Block *blocks[Traits::Memory::Page::ORDER + 1];
    } Heap;

    static void init();
    static void *kmalloc();
    static void kfree(void *);

    using PageList = LIFO<void>;
    using Page     = PageList::NodeType;
};

void *operator new(unsigned long, void *);
void *operator new(unsigned long, Memory::Role);
void operator delete(void *, unsigned long);
