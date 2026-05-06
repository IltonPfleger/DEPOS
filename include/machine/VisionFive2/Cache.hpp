#pragma once

#include <Traits.hpp>
#include <drivers/Driver.hpp>

namespace DEPOS {

class Cache : Driver {
    static constexpr unsigned long Base = Traits<MemoryMap>::CacheController;

    enum {
        L2_FLUSH           = 0x200,
        L2_CACHE_LINE_SIZE = 64,
    };

  public:
    static void flush(const void *const ptr, unsigned int size) {
        if (size == 0) return;

        unsigned long start = reinterpret_cast<unsigned long>(ptr);
        unsigned long line  = start & ~(static_cast<unsigned long>(L2_CACHE_LINE_SIZE) - 1);
        unsigned long end   = start + size;

        barrier();
        for (; line < end; line += L2_CACHE_LINE_SIZE) {
            Reg64(Base, L2_FLUSH) = line;
            barrier();
        }
    }

    static void barrier() { asm volatile("fence iorw, iorw" ::: "memory"); }
};

} // namespace DEPOS
