#pragma once

#include <architecture/rv/CPU.hpp>
#include <architecture/rv/Modes.hpp>
#include <drivers/Driver.hpp>

namespace rv64 {
class CLINT : Driver {
    enum Register {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static void handler(unsigned int) {
        if constexpr (!Meta::SAME<KernelMode, MachineMode>::Result) {
            csrc<MachineMode::IE>(MachineMode::TI);
            csrs<MachineMode::IP>(KernelMode::TI);
        } else {
            int core = CPU::id();
            CLINT::reset(core);
            Timer::handler(core);
        }
    }

    static void reset(unsigned int core = CPU::id()) {
        static constexpr unsigned long ticks = Clock / Traits<Timer>::Frequency;
        Reg64(Base, MTIMECMP + core * 8) = Reg64(Base, MTIME) + ticks;
    }

    static void init() {
        reset();
        csrs<MachineMode::IE>(MachineMode::TI);
    }
    static constexpr unsigned long Base = Traits<::CLINT>::Addr;
    static constexpr unsigned long Clock = Traits<::CLINT>::Clock;
    static constexpr bool Enable = Traits<::CLINT>::Enable;
};
} // namespace rv64
