#pragma once

#include <drivers/Driver.hpp>

template <unsigned long Base> class SiFiveCLINT : Driver {
    enum Register {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static void reset(unsigned long core) {
        static constexpr uintmax_t ticks = Traits<Clock>::CLINT / Traits<Timer>::Frequency;
        Reg64(Base, MTIMECMP + core * 8) = Reg64(Base, MTIME) + ticks;
    }
};
