#pragma once

#include <Traits.hpp>
#include <Types.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/VirtualPLIC.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>

namespace DEPOS {

namespace riscv64 {

class VirtualCPU {

    template <typename... Args>
    __attribute__((naked)) static void supervisor(Args... args) {
        ((void)args, ...);
        asm volatile("mret");
    }

  public:
    using Context = MachineContext;

    static constexpr unsigned long Address = Traits<MemoryMap>::PLIC;
    static constexpr unsigned long Size    = Traits<MemoryMap>::PLIC;

    template <typename... Args>
    VirtualCPU(void (*entry)(Args...), MemoryMap::Entry memory, Args... args)
        : m_mtimecmp(0),
          m_external_interrupt_pending(false),
          m_plic() {

        CPU::Interruptions::disable();

        csrw<SupervisorMode::SATP>(0);

        PMP::NAPOT<0>(memory.start, memory.end - memory.start,
                      PMP::R | PMP::W | PMP::X);

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

        csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR |
                                  MachineMode::PIRQE);
        csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);

        m_current = this;
        csrw<MachineMode::EPC>(entry);
        CPU::mb();
        supervisor(args...);
    }

    static auto current() { return m_current; }

    static void reset(unsigned long x) {
        csrc<MachineMode::IP>(SupervisorMode::TI);
        current()->m_mtimecmp = x;
    }

    static void handler() {
        if (!current()) return;
        if (CLINT::read() >= current()->m_mtimecmp)
            csrs<MachineMode::IP>(SupervisorMode::TI);
        if (current()->m_external_interrupt_pending) {
            current()->m_external_interrupt_pending = false;
            csrs<MachineMode::IP>(SupervisorMode::EI);
        }
    }

    void interrupt(unsigned int id) {
        if (m_plic.interrupt(id - 11)) m_external_interrupt_pending = true;
    }

    static bool read(unsigned long addr, unsigned int *destination) {
        return current()->m_plic.read(addr - Address, destination), true;
    }

    static bool write(unsigned long addr, unsigned int source) {
        return current()->m_plic.write(addr - Address, source);
    }

  private:
    static inline VirtualCPU *volatile m_current;

  private:
    unsigned long m_mtimecmp;
    bool m_external_interrupt_pending;
    VirtualPLIC m_plic;
};

} // namespace riscv64

} // namespace DEPOS
