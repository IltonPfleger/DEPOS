#pragma once

#include <Traits.hpp>

#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/VirtualPLIC.hpp>
#include <memory/Memory.hpp>
#include <memory/Segment.hpp>

namespace riscv64 {

class VirtualCPU {

    template <typename... Args> __attribute__((naked)) static void supervisor(Args... args) {
        (void(args), ...);
        asm volatile("mret");
    }

  public:
    using Context = MachineContext;

    static constexpr unsigned long Address = Traits<MemoryMap>::PLIC;
    static constexpr unsigned long Size = Traits<MemoryMap>::PLIC;

    template <typename... Args> VirtualCPU(void (*entry)(Args...), MemoryMap::Entry memory, Args... args) {
        CPU::Interruptions::disable();
        csrw<SupervisorMode::SATP>(0);
        // PMP::NAPOT<0>(Address, Size);
        PMP::NAPOT<0>(memory.start, memory.end - memory.start, PMP::R | PMP::W | PMP::X);
        //(void)memory;
        // PMP::TOR<1>(memory.start, memory.end, PMP::R | PMP::W | PMP::X);

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

        m_current = this;
        csrw<MachineMode::EPC>(entry);
        supervisor(args...);
    }

    static void reset(unsigned long x) {
        csrc<MachineMode::IP>(SupervisorMode::TI);
        m_current->m_mtimecmp = x;
    }

    static void handler(unsigned int id) {
        if (id == 7 && m_current && CLINT::read() >= m_current->m_mtimecmp) {
            csrs<MachineMode::IP>(SupervisorMode::TI);
        } else if (id > 11) {
            if (m_current && m_current->m_plic.interrupt(id - 11)) {
                csrs<MachineMode::IP>(SupervisorMode::EI);
            }
        }
    }

    static bool read(unsigned long addr, unsigned int *destination) {
        return m_current->m_plic.read(addr - Address, destination), true;
    }

    static bool write(unsigned long addr, unsigned int source) { return m_current->m_plic.write(addr - Address, source); }

  private:
    static inline VirtualCPU *m_current;
    unsigned long m_mtimecmp;
    VirtualPLIC m_plic;
};

} // namespace riscv64
