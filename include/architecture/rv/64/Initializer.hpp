#pragma once

#include <architecture/rv/CPU.hpp>
#include <architecture/rv/PLIC.hpp>
#include <architecture/rv/ic/IC.hpp>
#include <architecture/rv/ic/MIC.hpp>
#include <architecture/rv/ic/SIC.hpp>

namespace rv64 {
class Initializer {
    __attribute__((naked)) static void mode() {
        MIC::init();

        if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
            if (!(csrr<MachineMode::MISA>() & (1UL << ('S' - 'A')))) {
                CPU::halt();
            }
        }

        if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
            csrw<SupervisorMode::SATP>(0);
            csrw<MachineMode::MIDELEG>(0x222);
            csrw<MachineMode::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            csrw<MachineMode::PMPCFG0>(0b11111);
            csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
            csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);
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
        if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
            SIC::init();
        }
    }
};

} // namespace rv64
