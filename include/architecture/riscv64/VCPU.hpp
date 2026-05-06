#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/VPLIC.hpp>
#include <hypervisor/VirtualMachine.hpp>

namespace DEPOS {

namespace riscv64 {

class VCPU {
    template <typename... Args> __attribute__((naked)) static void supervisor(Args... args) {
        ((void)args, ...);
        asm volatile("mret");
    }

    static auto &current() { return s_current[CPU::id()]; }

  public:
    VCPU(VirtualMachine *vm, uintptr_t entry, size_t size)
        : m_entry(entry),
          m_size(size),
          m_mtimecmp(0),
          m_vm(vm) {}

    template <typename... Args> void start(Args... args) {
        CPU::Interrupt::disable();

        csrw<SupervisorMode::SATP>(0);

        PMP::NAPOT<0>(m_entry, m_size, PMP::R | PMP::W | PMP::X);

        long mideleg = 0;
        mideleg |= 1 << 1; // Supervisor Software Interrupt
        mideleg |= 1 << 5; // Supervisor Timer Interrupt
        mideleg |= 1 << 9; // Supervisor External Interrupt
        csrw<MachineMode::MIDELEG>(mideleg);

        long medeleg = 0;
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

        current() = this;
        csrw<MachineMode::EPC>(m_entry);
        CPU::mb();
        supervisor(args...);
    }

    static void onTick() {
        if (!current()) return;
        if (CLINT::read() >= current()->m_mtimecmp) csrs<MachineMode::IP>(SupervisorMode::TI);
        if (current()->m_plic.pending()) csrs<MachineMode::IP>(SupervisorMode::EI);
    }

    void interrupt(unsigned int id) { m_plic.interrupt(id); }

    static void reset(unsigned long x) {
        csrc<MachineMode::IP>(SupervisorMode::TI);
        current()->m_mtimecmp = x;
    }

    static bool read(unsigned long address, unsigned int *destination) {
        if (!current()) return false;
        if (current()->m_vm->read(address, destination)) {
            return true;
        } else {
            return current()->m_plic.read(address - Traits<MemoryMap>::PLIC, destination);
        }
    }

    static bool write(unsigned long address, unsigned int source) {
        if (!current()) return false;
        if (current()->m_vm->write(address, source)) {
            return true;
        } else {
            return current()->m_plic.write(address - Traits<MemoryMap>::PLIC, source);
        }
    }

  private:
    static inline VCPU *s_current[Traits<CPU>::Active];

  private:
    uintptr_t m_entry;
    size_t m_size;
    uintmax_t m_mtimecmp;
    VirtualMachine *m_vm;
    VPLIC m_plic;
};

} // namespace riscv64

} // namespace DEPOS
