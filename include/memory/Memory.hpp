#pragma once

#include <Traits.hpp>
#include <memory/Buddy.hpp>

class Memory {
   private:
    static constexpr size_t kPageOrder   = Traits::Memory::Page::ORDER;
    static constexpr size_t kPageSize    = Traits::Memory::Page::SIZE;
    static constexpr size_t kMemorySize  = Traits::Memory::SIZE;
    static constexpr size_t kMemoryOrder = Traits::Memory::ORDER;

    using Buddy = BuddyAllocator<kMemoryOrder>;

   public:
    static void init();
    static void* kmalloc(size_t = kPageSize);
    static void kfree(void*);

   private:
    static inline Buddy buddy_;
};

void* operator new(unsigned long, void*);
