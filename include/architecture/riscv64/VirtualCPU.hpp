#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/VirtualPLIC.hpp>
#include <drivers/hypervisor/VirtualMachine.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>

namespace DEPOS {

namespace riscv64 {

class VirtualCPU {

    template <typename... Args> __attribute__((naked)) static void supervisor(Args... args) {
        ((void)args, ...);
        asm volatile("mret");
    }

  public:
    using Context = MachineContext;

    static constexpr unsigned long Address = Traits<MemoryMap>::PLIC;
    static constexpr unsigned long Size    = Traits<MemoryMap>::PLIC;

    VirtualCPU() = default;
    template <typename... Args>
    VirtualCPU(void (*entry)(Args...), uintptr_t memory, size_t size, VirtualMachine *vm, Args... args)
        : m_mtimecmp(0),
          m_plic(),
          m_vm(vm) {

        CPU::Interruptions::disable();

        csrw<SupervisorMode::SATP>(0);

        PMP::NAPOT<0>(memory, size, PMP::R | PMP::W | PMP::X);

        unsigned long mideleg = 0;
        mideleg |= 1 << 1; // Supervisor Software Interrupt
        mideleg |= 1 << 5; // Supervisor Timer Interrupt
        mideleg |= 1 << 9; // Supervisor External Interrupt
        csrw<MachineMode::MIDELEG>(mideleg);

        unsigned long medeleg = 0;
        medeleg |= 1 << 3;  // Breakpoint
        medeleg |= 1 << 4;  // Load Address Misaligned
        medeleg |= 1 << 8;  // Environment Call From U-Mode
        medeleg |= 1 << 10; // Environment Call From VS-Mode
        medeleg |= 1 << 12; // Instruction Page Fault
        medeleg |= 1 << 13; // Load Page Fault
        medeleg |= 1 << 15; // Store Page Fault
        csrw<MachineMode::MEDELEG>(medeleg);

        csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
        csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);

        current(this);
        csrw<MachineMode::EPC>(entry);
        CPU::mb();
        supervisor(args...);
    }

    static auto current() { return s_current[CPU::id()]; }
    static void current(VirtualCPU *c) { s_current[CPU::id()] = c; }

    static void reset(unsigned long x) {
        csrc<MachineMode::IP>(SupervisorMode::TI);
        current()->m_mtimecmp = x;
    }

    static void handler() {
        if (!current()) return;
        if (CLINT::read() >= current()->m_mtimecmp) csrs<MachineMode::IP>(SupervisorMode::TI);
        if (current()->m_plic.pending()) csrs<MachineMode::IP>(SupervisorMode::EI);
    }

    void interrupt(unsigned int id) { m_plic.interrupt(id); }

    static bool read(unsigned long address, unsigned int *destination) {
        if (!current()) return false;
        if (current()->m_vm->read(address, destination)) {
            return true;
        } else {
            return current()->m_plic.read(address - Address, destination);
        }
    }

    static bool write(unsigned long address, unsigned int source) {
        if (!current()) return false;
        if (current()->m_vm->write(address, source)) {
            return true;
        } else {
            return current()->m_plic.write(address - Address, source);
        }
    }

  private:
    static inline VirtualCPU *volatile s_current[Traits<CPU>::Active];

  private:
    unsigned long m_mtimecmp;
    VirtualPLIC m_plic;
    VirtualMachine *m_vm;
};

} // namespace riscv64

} // namespace DEPOS
