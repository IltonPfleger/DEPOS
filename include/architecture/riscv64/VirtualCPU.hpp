#ifndef __DEPOS_RISCV64_VIRTUAL_CPU__
#define __DEPOS_RISCV64_VIRTUAL_CPU__

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/VirtualPLIC.hpp>
#include <hypervisor/VirtualMachine.hpp>

namespace DEPOS::riscv64 {

class VirtualCPU {
    template <typename... Args> __attribute__((naked)) static void supervisor(Args... args) {
        ((void)args, ...);
        asm volatile("mret");
    }

    static auto &current() { return s_current[CPU::id()]; }

  public:
    VirtualCPU(VirtualMachine *vm)
        : _mtimecmp(0),
          vm_(vm) {}

    template <typename... Args> void start(Args... args) {
        CPU::Interrupt::disable();

        csrw<SupervisorMode::SATP>(0);

        PMP::NAPOT<2>(vm_->memory().start(), vm_->memory().size(), PMP::R | PMP::W | PMP::X);

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

        core_     = mhartid();
        current() = this;
        csrw<MachineMode::EPC>(vm_->memory().start());
        CPU::mb();
        supervisor(args...);
    }

    void interrupt(unsigned int id) {
        plic_.interrupt(id);
        CLINT::ipi(core_);
    }

    static void onTick() {
        if (!current()) return;
        if (CLINT::mtime() >= current()->_mtimecmp) csrs<MachineMode::IP>(SupervisorMode::TI);
    }

    static void onInterProcessorInterrupt() {
        if (!current()) return;
        if (current()->plic_.pending()) csrs<MachineMode::IP>(SupervisorMode::EI);
    }

    static void reset(unsigned long r) {
        csrc<MachineMode::IP>(SupervisorMode::TI);
        current()->_mtimecmp = r;
    }

    static bool read(unsigned long address, unsigned int *destination) {
        if (!current()) return false;
        if (current()->vm_->read(address, destination)) {
            return true;
        } else {
            return current()->plic_.read(address - Traits<MemoryMap>::PLIC, destination);
        }
    }

    static bool write(unsigned long address, unsigned int source) {
        if (!current()) return false;
        if (current()->vm_->write(address, source)) {
            return true;
        } else {
            return current()->plic_.write(address - Traits<MemoryMap>::PLIC, source);
        }
    }

  private:
    static inline VirtualCPU *s_current[Traits<CPU>::Active];

  private:
    uintmax_t _mtimecmp;
    size_t core_;
    VirtualMachine *vm_;
    VirtualPLIC plic_;
};

} // namespace DEPOS::riscv64

#endif
