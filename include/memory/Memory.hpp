#pragma once

#include <memory/Buddy.hpp>
#include <Traits.hpp>

class Memory {
    using Buddy = BuddyAllocator<Traits::Memory::Page::ORDER>;

   public:
    static void init();
    static void* kmalloc();
    static void kfree(void*);

   private:
    static inline Buddy m_buddy;
};

void* operator new(unsigned long, void*);
