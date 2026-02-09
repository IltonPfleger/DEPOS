#pragma once

#include <architecture/common/Timer.hpp>
#include <architecture/riscv64/CLINT.hpp>

namespace riscv64 {

template <typename... Tickers> class Timer : public ArchitectureCommon::TimerTemplate<Tickers...> {

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
  public:
    static void init() {
        IC::bind(7, handler);
        csrs<MachineMode::IE>(MachineMode::TI);
        CLINT::write();
    }
};

} // namespace riscv64
