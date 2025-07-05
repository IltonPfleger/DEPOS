#pragma once

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
        unsigned long size;
        struct {
            unsigned char free : 1;
            unsigned char role : 7;
        } flags;
    } Block;

    struct Heap {
        Block *start;
    };

    static void init();
    static void *kmalloc();
    static void kfree(void *);
};

void *operator new(unsigned long, void *);
void *operator new(unsigned long, Memory::Role);
void operator delete(void *, unsigned long);
void operator delete(void *);
