#pragma once

#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/ic/Exception.hpp>

namespace rv {
class MIC {
    using Mode = MachineMode;
    using Context = ContextBase<Mode>;

    static void dispatch(Context *) {
        uintmax_t mcause = csrr<Mode::CAUSE>();
        int code = (mcause << 1) >> 1;

        if (mcause & IC::INTERRUPT) {
            IC::dispatch(code);
        } else {
            Exception<MachineMode>::dispatch();
        }
    }

  public:
    static void init() {
        if constexpr (CLINT::Enable) {
            CLINT::init();
            IC::bind(7, CLINT::handler);
        }

        if constexpr (PLIC::Enable) {
            PLIC::init();
            IC::bind(11, PLIC::handler);
        }
    }

    __attribute__((naked, aligned(4))) static void entry() {
        dispatch(Context::push());
        Context::pop();
    }
};

} // namespace rv

namespace rv64 {
using MIC = rv::MIC;
}
