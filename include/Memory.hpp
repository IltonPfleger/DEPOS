#pragma once

#include <Lists.hpp>
#include <Spin.hpp>
#include <Traits.hpp>

class Memory {
    using PageList = LIFO<void>;
    using Page     = PageList::NodeType;
    static PageList pages;

   public:
    static void init();
    static void *kmalloc();
    static void kfree(void *);
};

// void *operator new(unsigned long, void *);
// void *operator new(unsigned long, Memory::Role);
// void operator delete(void *, unsigned long);
