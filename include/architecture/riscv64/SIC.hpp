#pragma once

namespace riscv64 {

class SIC {
    static void dispatch(SupervisorContext *) {
        uintmax_t scause = csrr<SupervisorMode::CAUSE>();

        if (scause & IC::INTERRUPT) {
            IC::dispatch(scause & ~IC::INTERRUPT);
        } else {
            Exception<SupervisorMode>::dispatch();
        }
    }

    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        dispatch(SupervisorContext::push());
        SupervisorContext::pop();
    }

  public:
    static void init() { csrw<SupervisorMode::TVEC>(entry); }
};

} // namespace riscv64
