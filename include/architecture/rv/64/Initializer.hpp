#pragma once

#include <architecture/rv/64/MMU.hpp>
#include <architecture/rv/CPU.hpp>
#include <architecture/rv/PLIC.hpp>
#include <architecture/rv/ic/IC.hpp>
#include <architecture/rv/ic/MIC.hpp>
#include <architecture/rv/ic/SIC.hpp>

namespace rv64 {
class Initializer {
    __attribute__((naked)) static void mode() {
        MIC::init();
        csrs<MachineMode::IE>(~0U);

        if constexpr (Traits<RISCV>::Supervisor) {
            csrw<SupervisorMode::SATP>(0);
            csrw<MachineMode::MIDELEG>(0x1666);
            csrw<MachineMode::MEDELEG>(0xf4b509);
            csrw<MachineMode::PMPADDR0>(~0ULL);
            csrw<MachineMode::PMPCFG0>(0x1f);
            csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
            csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);

            if constexpr (Traits<System>::Multitask) {
                SV39_MMU::init();
            }

        } else {
            csrs<MachineMode::STATUS>(MachineMode::ME2ME);
            csrc<MachineMode::STATUS>(MachineMode::PIRQE);
        }

        csrw<MachineMode::EPC>(__builtin_return_address(0));
        MachineMode::ret();
    }

  public:
    static void init() {
        mode();
        if constexpr (Traits<RISCV>::Supervisor) {
            SIC::init();
        }
    }
};
} // namespace rv64
