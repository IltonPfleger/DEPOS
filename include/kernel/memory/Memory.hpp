#pragma once

#include <Traits.hpp>

#include <shared/memory/allocators/Buddy.hpp>
#include <shared/synchronization/Spin.hpp>

namespace DEPOS {

class Memory {
  private:
    using Allocator = allocators::Buddy<Traits<Memory>::Order - 1>;

  public:
    static void init();
    static void *alloc(size_t);
    static void free(void *, size_t);
    static uintptr_t virt2phys(uintptr_t);

  private:
    static inline Allocator s_allocator;
    static inline Spin s_spin;
};

} // namespace DEPOS
