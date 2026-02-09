#pragma once

#include <architecture/riscv64/ic/ReducedSBI.hpp>

namespace riscv64 {

class SIC {
    using Context = SupervisorContext;

    // static void timer(unsigned int) {
    //     CPU::syscall(0, 0, 0, 0, 0, 0, 0, ReducedSBI::TIME);
    //     Timer::handler(CPU::id());
    // }

    static void dispatch(Context *) {
        uintmax_t scause = csrr<SupervisorMode::CAUSE>();

        if (scause & IC::INTERRUPT) {
            IC::dispatch(scause & ~IC::INTERRUPT);
        } else {
            Exception<SupervisorMode>::dispatch();
        }
    }

    __attribute__((naked, aligned(4))) static void entry() {
        dispatch(Context::push());
        Context::pop();
    }

  public:
    static void init() {
        csrw<SupervisorMode::TVEC>(entry);

        // if constexpr (Traits<Timer>::Enable) {
        //     IC::bind(5, timer);
        //     csrs<SupervisorMode::IE>(SupervisorMode::TI);
        // }

        // if constexpr (PLIC::Enable) {
        //     csrs<KernelMode::IE>(KernelMode::EI);
        // }
    }
};

} // namespace riscv64
