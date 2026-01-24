#pragma once

#include <Traits.hpp>
#include <machine/Traits.hpp>
#include <memory/allocators/Buddy.hpp>

class Memory {
  private:
    using Allocator = Allocators::Buddy<Traits<Memory>::Order>;

  public:
    static void init();
    static void *kmalloc(size_t);
    static void kfree(void *, size_t);

  private:
    static inline Allocator s_allocator;
};

void *operator new(size_t, void *);
