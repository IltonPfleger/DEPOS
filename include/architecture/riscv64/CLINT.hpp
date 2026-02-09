#pragma once

#include <architecture/common/Timer.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/Traits.hpp>
#include <drivers/Driver.hpp>
#include <utils/Debug.hpp>

namespace riscv64 {

template <typename... Tickers> class CLINT : Driver, ArchitectureCommon::TimerTemplate<Tickers...> {

    enum Registers {
        MTIMECMP = 0x4000,
        MTIME = 0xBFF8,
    };

  public:
    static uint64_t read() { return Reg64(Addr, MTIME); }

    static void write(uint64_t ticks = read() + Ticks, unsigned int core = csrr<MachineMode::HARTID>()) {
        Reg64(Addr, MTIMECMP + core * 8) = ticks;
    }

    static void handler(unsigned int) {
        CLINT::write();
        ArchitectureCommon::TimerTemplate<Tickers...>::handler(CPU::id());
    }

    //    //     if constexpr (Traits<RISCV>::Supervisor) {
    //    //         csrc<MachineMode::IE>(MachineMode::TI);
    //    //         csrs<MachineMode::IP>(SupervisorMode::TI);
    //    //     } else {
    //    //         CLINT::write();
    //    //         Timer::handler(CPU::id());
    //    //     }
    //    // }
    //
    //    // static void syscall(uint64_t delta = 0) {
    //    //     if (delta == 0) delta = Ticks + read();
    //    //     write(delta);
    //    //     csrs<MachineMode::IE>(MachineMode::TI);
    //    //     csrc<MachineMode::IP>(SupervisorMode::TI);
    //    // }
    //
    static void init() {
        IC::bind(7, handler);
        csrs<MachineMode::IE>(MachineMode::TI);
        write();
    }

  public:
    static constexpr unsigned long Addr = Traits<::CLINT>::Addr;
    static constexpr unsigned long Clock = Traits<::CLINT>::Clock;
    static constexpr unsigned long Ticks = Clock / Traits<Timer>::Frequency;
};

} // namespace riscv64
