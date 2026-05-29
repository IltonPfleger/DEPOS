#ifndef __DEPOS_RISCV64_VCPU__
#define __DEPOS_RISCV64_VCPU__

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/VPLIC.hpp>
#include <hypervisor/VirtualMachine.hpp>

namespace DEPOS::riscv64 {

class VCPU {
  public:
    VCPU(VirtualMachine *vm)
        : mtimecmp_(0),
          first_(true),
          vm_(vm) {}

    static VCPU *current() { return current_[CPU::id()]; }
    static void current(VCPU *self) { current_[CPU::id()] = self; }

    void activate(auto... args) {
        bool enabled = CPU::Interrupt::disable();

        PMP::NAPOT<2>(vm_->memory().start(), vm_->memory().size(), PMP::R | PMP::W | PMP::X);

        csrw<MachineMode::MIDELEG>(MIDELEG);
        csrw<MachineMode::MEDELEG>(MEDELEG);

        core_ = csrr<MachineMode::HARTID>();
        current(this);

        if (first_) {
            first_ = false;
            csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
            csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);
            csrw<MachineMode::EPC>(vm_->memory().start());
            csrw<SupervisorMode::SATP>(0);
            dispatch(args...);
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
    __attribute__((naked)) static void dispatch(auto... args) {
        (static_cast<void>(args), ...);
        asm("mret");
    }

  private:
    static constexpr uintmax_t MIDELEG = SupervisorMode::SI | SupervisorMode::TI | SupervisorMode::EI;
    static constexpr uintmax_t MEDELEG = 1 << 3     // Breakpoint
                                         | 1 << 4   // Load Address Misaligned
                                         | 1 << 8   // Environment Call From U-Mode
                                         | 1 << 10  // Environment Call From VS-Mode
                                         | 1 << 12  // Instruction Page Fault
                                         | 1 << 13  // Load Page Fault
                                         | 1 << 15; // Store Page Fault

  private:
    static constinit inline VCPU *current_[Traits<CPU>::Active] = {nullptr};

  private:
    uintmax_t mtimecmp_;
    size_t core_;
    bool first_;
    VirtualMachine *vm_;
    VPLIC plic_;
};

} // namespace DEPOS::riscv64

#endif
