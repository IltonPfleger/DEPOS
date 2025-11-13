#pragma once

#include <Traits.hpp>
#include <memory/Buddy.hpp>

class Memory {
   private:
    using Buddy = BuddyAllocator<Traits::Memory::ORDER>;

   public:
    static void init();
    static void* kmalloc(size_t);
    static void kfree(void*, size_t);

   private:
    static inline Buddy buddy_;
};

void *operator new(unsigned long, void *);
