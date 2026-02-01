#pragma once

#include <Timer.hpp>
#include <architecture/rv/ic/ReducedSBI.hpp>

namespace rv {
class SIC {
    using Mode = SupervisorMode;
    using Context = ContextBase<Mode>;

    static void timer(unsigned int) {
        CPU::syscall(ReducedSBI::TIME);
        Timer::handler(CPU::id());
    }

    static void dispatch(Context *) {
        uintmax_t scause = csrr<Mode::CAUSE>();
        int code = (scause << 1) >> 1;

        if (scause & IC::INTERRUPT) {
            IC::dispatch(code);
        } else {
            Exception<Mode>::dispatch();
        }
    }

    __attribute__((naked, aligned(4))) static void entry() {
        dispatch(Context::push());
        Context::pop();
    }

  public:
    static void init() {
        csrw<Mode::TVEC>(entry);
        if constexpr (Traits<Timer>::Enable) {
            csrs<Mode::IE>(Mode::TI);
            IC::bind(5, timer);
        }
    }
};

} // namespace rv
