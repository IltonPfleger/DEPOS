#pragma once

#include <architecture/common/Timer.hpp>
#include <architecture/riscv64/CLINT.hpp>

namespace riscv64 {

template <typename... Tickers> class Timer : public ArchitectureCommon::TimerTemplate<Tickers...> {

    static void machine(unsigned int) {
        CLINT::write();
        ArchitectureCommon::TimerTemplate<Tickers...>::handler(CPU::id());
    }

    static void supervisor(unsigned int) {
        CPU::syscall(0, 0, 0, 0, 0, 0, 0, 0);
        ArchitectureCommon::TimerTemplate<Tickers...>::handler(CPU::id());
    }

  public:
    static void init() {
        if constexpr (!Traits<RISCV>::Supervisor) {
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
