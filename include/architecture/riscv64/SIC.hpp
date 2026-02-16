#pragma once

namespace riscv64 {

class SIC {
    static void dispatch(SupervisorContext *) {
        uintmax_t scause = csrr<SupervisorMode::CAUSE>();

        if (scause & IC::INTERRUPT) {
            unsigned int id = scause & ~IC::INTERRUPT;

            if (id == 9) {
                id = PLIC::claim();
                if (id) IC::dispatch(id + 11);
                PLIC::complete(id);
            } else {
                IC::dispatch(id);
            }
        } else {
            Exception<SupervisorMode>::dispatch();
        }
    }

    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        dispatch(SupervisorContext::push());
        SupervisorContext::pop();
    }

  public:
    static void init() {
        csrw<SupervisorMode::TVEC>(entry);

        if constexpr (Traits<RISCV>::Supervisor && Traits<::PLIC>::Enable) {
            PLIC::init();
            csrs<SupervisorMode::IE>(SupervisorMode::EI);
        }
    }
};

} // namespace riscv64
