#pragma once

#include <architecture/common/Timer.hpp>
#include <architecture/riscv64/CLINT.hpp>

namespace riscv64 {

template <typename... Tickers> class Timer : public ArchitectureCommon::TimerTemplate<Tickers...> {

    static void machine(unsigned int) {
        if constexpr (Traits<RISCV>::Supervisor) {
            csrc<MachineMode::IE>(MachineMode::TI);
            csrs<MachineMode::IP>(SupervisorMode::TI);
        } else {
            CLINT::write();
            ArchitectureCommon::TimerTemplate<Tickers...>::handler(CPU::id());
        }
    }

    static void supervisor(unsigned int) {
        CPU::syscall(0, 0, 0, 0, 0, 0, 0, ReducedSBI::TIME);
        ArchitectureCommon::TimerTemplate<Tickers...>::handler(CPU::id());
    }

  public:
    template <bool mode = Traits<RISCV>::Supervisor ? 0 : 1> static void init() {
        if (mode) {
            IC::bind(7, machine);
            csrs<MachineMode::IE>(MachineMode::TI);
            CLINT::write();
        } else {
            IC::bind(5, supervisor);
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
        }
    }
};

} // namespace riscv64
