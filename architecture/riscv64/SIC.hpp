#pragma once

#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/TrapHandler.hpp>

namespace DEPOS {

class SIC {

  public:
    static void init() {
        TrapHandler::init<SupervisorMode, Traits<Thread>::IsolatedKernelStack>();

        if constexpr (Traits<PLIC>::Enable) {
            PLIC::init();
            TrapHandler::install(9, IC::onTrap);
            csrs<SupervisorMode::IE>(SupervisorMode::EI);
        }
    }
};

} // namespace DEPOS
