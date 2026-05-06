#pragma once

#include <Traits.hpp>
#include <drivers/Driver.hpp>

namespace DEPOS {

template <typename> class SiFiveU74L2CacheController : Driver {

    enum { FLUSH_64 = 0x200, WAY_ENABLE = 0x8 };

  public:
    static void flush(const void *const ptr, size_t size) {
        if (size == 0) return;

        uintptr_t start = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t line  = start & ~(static_cast<uintptr_t>(CacheLineSize) - 1);
        uintptr_t end   = start + size;

        barrier();
        for (; line < end; line += CacheLineSize) {
            Reg64(Address, FLUSH_64) = line;
            barrier();
        }
    }

    template <typename... Args> static void invalidate(Args... args) { flush(args...); }

    static void barrier() { asm volatile("fence iorw, iorw" ::: "memory"); }

    static void init() { Reg8(Address, WAY_ENABLE) = 0xff; }

  private:
    static constexpr uintptr_t Address    = 0x2010000;
    static constexpr size_t CacheLineSize = 64;
};

} // namespace DEPOS
