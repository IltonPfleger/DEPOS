#pragma once

#include <architecture/rv/CPU.hpp>
#include <architecture/rv/PLIC.hpp>
#include <architecture/rv/ic/IC.hpp>
#include <architecture/rv/ic/MIC.hpp>
#include <architecture/rv/ic/SIC.hpp>

namespace rv64 {
class Initializer {
    __attribute__((naked)) static void change_to_main_mode() {
        if constexpr (Traits<System>::MULTITASK) {
            if (!(csrr<MachineMode::MISA>() & (1UL << ('S' - 'A')))) {
                for (;;)
                    CPU::idle();
            }
        }

        MIC::init();
        csrw<MachineMode::TVEC>(MIC::entry);

        if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
            csrw<SupervisorMode::TVEC>(SIC::entry);
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
        __builtin_unreachable();
    }

  public:
    __attribute__((noinline)) static void init() {
        change_to_main_mode();
        if constexpr (Traits<Timer>::Enable && Traits<System>::MULTITASK) {
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
        }
    }
};

} // namespace rv64
