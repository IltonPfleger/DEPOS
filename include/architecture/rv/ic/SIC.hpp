#pragma once

#include <Timer.hpp>
#include <architecture/rv/ic/ReducedSBI.hpp>

namespace rv {
class SIC {
    using Context = ContextBase<SupervisorMode>;

    static void timer(unsigned int) {
        CPU::syscall(0, 0, 0, 0, 0, 0, 0, ReducedSBI::TIME);
        Timer::handler(CPU::id());
    }

    static void dispatch(Context *) {
        uintmax_t scause = csrr<SupervisorMode::CAUSE>();
        int code = scause & ~IC::INTERRUPT;

        if (scause & IC::INTERRUPT) {
            IC::dispatch(code);
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

        if constexpr (Traits<Timer>::Enable) {
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
            IC::bind(5, timer);
        }
    }
};

} // namespace rv
