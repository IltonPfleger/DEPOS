#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/Traits.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <drivers/Driver.hpp>

namespace DEPOS {

class CLINT : Driver {
    enum Registers {
        MTIMECMP = 0x4000,
        MTIME    = 0xBFF8,
    };

  public:
    static uint64_t mtime() { return Reg64(Address, MTIME); }

    static void ipi(size_t hartid, bool clear = false) {
        *(reinterpret_cast<volatile uint32_t *>(Address) + hartid) = !clear;
    }

    static void write(uint64_t ticks = mtime() + Ticks, unsigned int core = csrr<MachineMode::HARTID>()) {
        Reg64(Address, MTIMECMP + core * 8) = ticks;
    }

    static void forward(unsigned int = 0) {
        csrc<MachineMode::IE>(MachineMode::TI);
        csrs<MachineMode::IP>(SupervisorMode::TI);
    }

    static void syscall(uint64_t delta = 0) {
        if (delta == 0) delta = Ticks + mtime();
        write(delta);
        csrc<MachineMode::IP>(SupervisorMode::TI);
        csrs<MachineMode::IE>(MachineMode::TI);
    }

  public:
    static constexpr unsigned long Address = Traits<CLINT>::Address;
    static constexpr unsigned long Clock   = Traits<CLINT>::Clock;
    static constexpr unsigned long Ticks   = Clock / Traits<DEPOS::Timer>::Frequency;
};

} // namespace DEPOS
