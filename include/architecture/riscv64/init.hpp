#pragma once

#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/MMU.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <architecture/riscv64/PMP.hpp>
// #include <architecture/riscv64/VirtualCPU.hpp>
#include <architecture/riscv64/ic/IC.hpp>
#include <architecture/riscv64/ic/MIC.hpp>
// #include <architecture/riscv64/ic/SIC.hpp>

namespace riscv64 {
//__attribute__((naked)) static void supervisor() {
//    if constexpr (Traits<RISCV>::Supervisor) {
//        csrw<SupervisorMode::SATP>(0);
//        PMP::NAPOT<2>(0, 0, PMP::R | PMP::W | PMP::X);
//        csrw<MachineMode::MIDELEG>(0x1666);
//        csrw<MachineMode::MEDELEG>(0xf4b509);
//        csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
//        csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);
//    }

//    csrw<MachineMode::EPC>(__builtin_return_address(0));
//    MachineMode::ret();
//}

inline void init() {
    csrw<MachineMode::IE>(0);

    // PMP::TOR<1>(__kmm.text.start, __kmm.text.end, PMP::X | PMP::LOCK);

    MIC::init();

    if constexpr (Traits<RISCV>::Supervisor) {
        // new (Memory::alloc(sizeof(VirtualCPU))) VirtualCPU();
        // SIC::init();
    }

    // mode();
    // if constexpr (Traits<System>::Multitask) {
    //    SV39_MMU::init();
    // }
    // if constexpr (Traits<RISCV>::Supervisor) {
    // }
}
} // namespace riscv64
