#pragma once

#include <Traits.hpp>
#include <drivers/Driver.hpp>

class CacheController : Driver {
    static constexpr unsigned long Base = Traits<MemoryMap>::CacheController;

    enum {
        L2_FLUSH = 0x200,
        L2_CACHE_LINE_SIZE = 64,
    };

  public:
    static void flush(const void *const ptr, unsigned int size) {
        unsigned long line = reinterpret_cast<unsigned long>(ptr);
        unsigned long end = line + size;
        barrier();
        for (; line < end; line += L2_CACHE_LINE_SIZE) {
            Reg64(Base, L2_FLUSH) = line;
            barrier();
        }
    }

    static void barrier() { asm volatile("fence iorw, iorw" ::: "memory"); }
};
