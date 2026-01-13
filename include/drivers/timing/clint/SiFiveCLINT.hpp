#pragma once

#include <drivers/Driver.hpp>

class SiFiveCLINT : Driver {
    static constexpr unsigned long Base = Traits<MemoryMap>::CLINT;

    enum Register {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static void reset(unsigned int core) {
        static constexpr unsigned long ticks = Traits<Clock>::CLINT / Traits<Timer>::Frequency;
        Reg64(Base, MTIMECMP + core * 8) = Reg64(Base, MTIME) + ticks;
    }
};
