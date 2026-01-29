#pragma once

#include <architecture/rv/CPU.hpp>
#include <architecture/rv/Modes.hpp>
#include <architecture/rv/Traits.hpp>
#include <drivers/Driver.hpp>

namespace rv64 {
class CLINT : Driver {
    enum Register {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static uint64_t read() { return Reg64(Base, MTIME); }

    static void write(uint64_t ticks = read() + Ticks, unsigned int core = csrr<MachineMode::HARTID>()) {
        Reg64(Base, MTIMECMP + core * 8) = ticks;
    }

    static void handler(unsigned int) {
        if constexpr (Traits<RISCV>::Supervisor) {
            csrc<MachineMode::IE>(MachineMode::TI);
            csrs<MachineMode::IP>(KernelMode::TI);
        } else {
            CLINT::write();
            Timer::handler(CPU::id());
        }
    }

    static void syscall() {
        write();
        csrs<MachineMode::IE>(MachineMode::TI);
        csrc<MachineMode::IP>(KernelMode::TI);
    }

    static void init() {
        write();
        csrs<MachineMode::IE>(MachineMode::TI);
    }
    static constexpr unsigned long Base = Traits<::CLINT>::Addr;
    static constexpr unsigned long Clock = Traits<::CLINT>::Clock;
    static constexpr unsigned long Ticks = Clock / Traits<Timer>::Frequency;
    static constexpr bool Enable = Traits<::CLINT>::Enable;
};
} // namespace rv64
