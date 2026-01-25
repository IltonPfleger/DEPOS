#pragma once

#include <Spin.hpp>
#include <Traits.hpp>
#include <machine/Traits.hpp>
#include <memory/MemoryMap.hpp>
#include <memory/allocators/Buddy.hpp>

class Memory {
  private:
    using Allocator = Allocators::Buddy<Traits<Memory>::Order>;

  public:
    static void init();
    static void *alloc(size_t);
    static void free(void *, size_t);
    static uintptr_t virt2phys(uintptr_t);

  private:
    static inline Allocator s_allocator;
    static inline Spin s_spin;
};

void *operator new(size_t, void *);
