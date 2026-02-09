#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <drivers/Driver.hpp>

namespace riscv64 {

class CLINT : Driver {
    enum Registers {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static uint64_t read() { return Reg64(Addr, MTIME); }

    static void write(uint64_t ticks = read() + Ticks, unsigned int core = csrr<MachineMode::HARTID>()) {
        Reg64(Addr, MTIMECMP + core * 8) = ticks;
    }

  public:
    static constexpr unsigned long Addr = Traits<::CLINT>::Addr;
    static constexpr unsigned long Clock = Traits<::CLINT>::Clock;
    static constexpr unsigned long Ticks = Clock / Traits<Timer>::Frequency;
};

} // namespace riscv64
