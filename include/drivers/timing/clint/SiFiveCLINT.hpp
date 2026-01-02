#pragma once

#include <drivers/Driver.hpp>

class SiFiveCLINT : Driver {
    static constexpr unsigned long Base = Traits<MemoryMap>::CLINT;

    enum Register {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static void reset(unsigned long core) {
        static constexpr uintmax_t ticks = Traits<Timer>::Frequency;
        Reg64(Base, MTIMECMP + core * sizeof(unsigned long)) = Reg64(Base, MTIME) + ticks;
    }
};
