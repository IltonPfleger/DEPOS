#pragma once

#include <architecture/rv/CPU.hpp>
#include <drivers/Driver.hpp>

namespace rv64 {
class CLINT : Driver {
    static constexpr unsigned long Base = Traits<::CLINT>::Addr;
    static constexpr unsigned long Clock = Traits<::CLINT>::Clock;
    static constexpr bool Enable = Traits<::CLINT>::Enable;

    enum Register {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static void reset(unsigned int core = CPU::id()) {
        static constexpr unsigned long ticks = Clock / Traits<Timer>::Frequency;
        Reg64(Base, MTIMECMP + core * 8) = Reg64(Base, MTIME) + ticks;
    }

    static void init() {
        if constexpr (Enable) {
            reset();
            csrs<MachineMode::IE>(MachineMode::TI);
        }
    }
};
} // namespace rv64
