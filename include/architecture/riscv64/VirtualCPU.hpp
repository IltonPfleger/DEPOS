#ifndef __DEPOS_RISCV64_VIRTUAL_CPU__
#define __DEPOS_RISCV64_VIRTUAL_CPU__

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/VirtualPLIC.hpp>
#include <hypervisor/VirtualMachine.hpp>
#include <utility/Console.hpp>

namespace DEPOS::riscv64 {

class VirtualCPU {
    template <typename... Args> __attribute__((naked)) static void supervisor(Args... args) {
        ((void)args, ...);
        asm volatile("mret");
    }

  public:
    VirtualCPU(VirtualMachine *vm)
        : mtimecmp_(0),
          first_(true),
          vm_(vm) {}

    static VirtualCPU *current() { return current_[CPU::id()]; }
    static void current(VirtualCPU *self) { current_[CPU::id()] = self; }

    template <typename... Args> void activate(Args... args) {
        bool enabled = CPU::Interrupt::disable();

        PMP::NAPOT<2>(vm_->memory().start(), vm_->memory().size(), PMP::R | PMP::W | PMP::X);

        uintmax_t mideleg = 0;
        mideleg |= 1 << 1; // Supervisor Software Interrupt
        mideleg |= 1 << 5; // Supervisor Timer Interrupt
        mideleg |= 1 << 9; // Supervisor External Interrupt
        csrw<MachineMode::MIDELEG>(mideleg);

        uintmax_t medeleg = 0;
        medeleg |= 1 << 3;  // Breakpoint
        medeleg |= 1 << 4;  // Load Address Misaligned
        medeleg |= 1 << 8;  // Environment Call From U-Mode
        medeleg |= 1 << 10; // Environment Call From VS-Mode
        medeleg |= 1 << 12; // Instruction Page Fault
        medeleg |= 1 << 13; // Load Page Fault
        medeleg |= 1 << 15; // Store Page Fault
        csrw<MachineMode::MEDELEG>(medeleg);

        core_ = csrr<MachineMode::HARTID>();
        current(this);

        if (first_) {
            first_ = false;
            csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
            csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);
            csrw<MachineMode::EPC>(vm_->memory().start());
            csrw<SupervisorMode::SATP>(0);
            supervisor(args...);
        }

        if (enabled) CPU::Interrupt::enable();
    }

    void interrupt(unsigned int id) {
        plic_.interrupt(id);
        CLINT::ipi(core_);
    }

    static void onTick() {
        if (!current()) return;
        if (CLINT::mtime() >= current()->mtimecmp_) doTimerInterrupt();
    }

    static void onInterProcessorInterrupt() {
        if (!current()) return;
        if (current()->plic_.pending()) doExternalInterrupt();
    }

    static void reset(unsigned long r) {
        csrc<MachineMode::IP>(SupervisorMode::TI);
        current()->mtimecmp_ = r;
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
    static void doTimerInterrupt() { csrs<MachineMode::IP>(SupervisorMode::TI); }
    static void doExternalInterrupt() { csrs<MachineMode::IP>(SupervisorMode::EI); }

  private:
    static constinit inline VirtualCPU *current_[Traits<CPU>::Active] = {nullptr};

  private:
    uintmax_t mtimecmp_;
    size_t core_;
    bool first_;
    VirtualMachine *vm_;
    VirtualPLIC plic_;
};

} // namespace DEPOS::riscv64

#endif
