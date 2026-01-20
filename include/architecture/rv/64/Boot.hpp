#pragma once

#include <architecture/rv/CPU.hpp>
#include <architecture/rv/IC.hpp>
#include <architecture/rv/PLIC.hpp>

namespace rv64 {
class Boot {

    __attribute__((naked)) static void jmode() {
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
    __attribute__((naked)) static void probe() {
        unsigned long core;
        asm("csrr tp, mhartid\nmv %0, tp" : "=r"(core));
        asm("mv sp, %0" ::"r"(Traits<MemoryMap>::RamEnd - Traits<Memory>::PageSize * core));
        asm("ret");
    }

    __attribute__((noinline)) static void init() {
        jmode();
        if constexpr (Traits<Timer>::Enable && Traits<System>::MULTITASK) {
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
        }
    }
};

} // namespace rv64
