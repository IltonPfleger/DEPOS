#pragma once

#include <drivers/Driver.hpp>

template <unsigned long Base> class SiFiveCLINT : Driver {
    enum Register {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static unsigned long now() { return Reg64(Base, MTIME); }

    static void reset(unsigned int core) {
        static constexpr unsigned long ticks = Traits<Clock>::CLINT / Traits<Timer>::Frequency;
        Reg64(Base, MTIMECMP + core * 8) = now() + ticks;
    }
};
