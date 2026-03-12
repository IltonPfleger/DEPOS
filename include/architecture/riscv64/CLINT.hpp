#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/Traits.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <drivers/Driver.hpp>

namespace DEPOS {

namespace riscv64 {

class CLINT : Driver {
    enum Registers {
        MTIMECMP = 0x4000,
        MTIME    = 0xBFF8,
    };

  public:
    static uint64_t read() { return Reg64(Addr, MTIME); }

    static void write(uint64_t ticks    = read() + Ticks,
                      unsigned int core = csrr<MachineMode::HARTID>()) {
        Reg64(Addr, MTIMECMP + core * 8) = ticks;
    }

    static void forward(unsigned int = 0) {
        csrc<MachineMode::IE>(MachineMode::TI);
        csrs<MachineMode::IP>(SupervisorMode::TI);
    }

    static void syscall(uint64_t delta = 0) {
        if (delta == 0) delta = Ticks + read();
        write(delta);
        csrc<MachineMode::IP>(SupervisorMode::TI);
        csrs<MachineMode::IE>(MachineMode::TI);
    }

  public:
    static constexpr unsigned long Addr  = Traits<CLINT>::Addr;
    static constexpr unsigned long Clock = Traits<CLINT>::Clock;
    static constexpr unsigned long Ticks = Clock / Traits<Timer>::Frequency;
};

} // namespace riscv64

} // namespace DEPOS
