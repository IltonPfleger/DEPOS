#pragma once

#include <architecture/common/Timer.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/IC.hpp>

namespace DEPOS {

namespace riscv64 {

template <typename... Tickers> class Timer : public ArchitectureCommon::TimerTemplate<Tickers...> {

    static void machine(unsigned int) {
        CLINT::write();
        ArchitectureCommon::TimerTemplate<Tickers...>::handler(CPU::id());
    }

    static void supervisor(unsigned int) {
        CPU::syscall();
        ArchitectureCommon::TimerTemplate<Tickers...>::handler(CPU::id());
    }

  public:
    static inline uint64_t now() { return CLINT::read() * 1'000'000 / Traits<CLINT>::Clock; }

    static void init() {
        if constexpr (!Traits<RISCV>::Supervisor) {
            IC::bind(7, machine, true, false);
            csrs<MachineMode::IE>(MachineMode::TI);
            CLINT::write();
        } else {
			IC::bind(5, supervisor, true, false);
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
        }
    }
};

} // namespace riscv64

} // namespace DEPOS
