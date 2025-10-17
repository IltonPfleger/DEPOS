#pragma once

#include <Lists.hpp>
#include <Traits.hpp>

class Memory {
   public:
    static void init();
    static void *kmalloc();
    static void kfree(void *);

   private:
    using PageList = LIFO<void>;
    using Page     = PageList::NodeType;
    static inline PageList pages;

   public:


};

// void *operator new(unsigned long, void *);
// void *operator new(unsigned long, Memory::Role);
// void operator delete(void *, unsigned long);
