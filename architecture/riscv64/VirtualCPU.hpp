#ifndef __DEPOS_RISCV64_VCPU__
#define __DEPOS_RISCV64_VCPU__

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/MMU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <hypervisor/VirtualMachine.hpp>
#include <utility/Debug.hpp>

namespace DEPOS {

class VirtualCPU {
  public:
    VirtualCPU(VirtualMachine *vm)
        : mtimecmp_(0),
          sip_(0),
          core_(-1),
          vm_(vm) {}

    void boot(auto... args) {
        CPU::Interrupt::disable();

        activate();

        csrs<MachineMode::STATUS>(MachineMode::PP_S | MachineMode::PIRQE);
        csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);
        csrw<MachineMode::EPC>(vm_->memory().start());
        csrw<SupervisorMode::SATP>(0);
        dispatch(args...);
    }

    void activate() {
        MMU::TLB::flush();

        PMP::NAPOT<1>(vm_->memory().start(), vm_->memory().size(), PMP::R | PMP::W | PMP::X);

        csrw<MachineMode::MIDELEG>(MIDELEG);
        csrw<MachineMode::MEDELEG>(MEDELEG);

        core_ = csrr<MachineMode::HARTID>();
        current(this);

        sip(sip_);

        onTick();
    }

    void setInterruptPending() {
        sip_ |= SupervisorMode::EI;
        if (current() == this) {
            sip(this->sip_);
        } else if (core_ > 0) {
            CLINT::ipi(core_);
        }
    }

    void clearInterruptPending() {
        sip_ &= ~SupervisorMode::EI;
        if (current() == this) {
            sip(this->sip_);
        } else if (core_ > 0) {
            CLINT::ipi(core_);
        }
    }

    static void onInterProcessorInterrupt() { onExternalInterrupt(); }

    static void onTick() {
        if (!current()) return;
        if (CLINT::mtime() >= current()->mtimecmp_) setTimerInterruptPending();
    }

    static void onExternalInterrupt() {
        if (!current()) return;
        sip(current()->sip_);
    }

    static uintmax_t mtimecmp() {
        assert(current());
        return current()->mtimecmp_;
    }

    static void mtimecmp(uintmax_t mtimecmp) {
        assert(current());
        current()->mtimecmp_ = mtimecmp;
        if (mtimecmp > CLINT::mtime()) {
            clearTimerInterruptPending();
        } else {
            setTimerInterruptPending();
        }
    }

    static bool read(uintptr_t address, uint32_t *destination) {
        if (!current()) return false;
        return current()->vm_->read(address, destination);
    }

    static bool write(uintptr_t address, uint32_t source) {
        if (!current()) return false;
        return current()->vm_->write(address, source);
    }

    static VirtualCPU *swap(VirtualCPU *next) {
        VirtualCPU *previous = current();
        if (next) {
            next->activate();
        } else {
            current(nullptr);
        }
        if (previous) {
            previous->sip_  = csrr<MachineMode::IP>();
            previous->core_ = -1;
        }
        return previous;
    }

  private:
    __attribute__((naked)) static void dispatch(auto... args) {
        (static_cast<void>(args), ...);
        asm("mret");
    }

    static void setTimerInterruptPending() { csrs<MachineMode::IP>(SupervisorMode::TI); }
    static void clearTimerInterruptPending() { csrc<MachineMode::IP>(SupervisorMode::TI); }
    static void current(VirtualCPU *current) { current_[CPU::id()] = current; }
    static VirtualCPU *current() { return current_[CPU::id()]; }
    static void sip(uintmax_t sip) {
        csrc<MachineMode::IP>(0x202);
        csrs<MachineMode::IP>(sip & 0x202);
    }

  private:
    static constinit inline VirtualCPU *current_[Traits<CPU>::Active];
    static constexpr uintmax_t MIDELEG = SupervisorMode::SI | SupervisorMode::TI | SupervisorMode::EI;
    static constexpr uintmax_t MEDELEG = 1 << 3     // Breakpoint
                                         | 1 << 4   // Load Address Misaligned
                                         | 1 << 8   // Environment Call From U-Mode
                                         | 1 << 10  // Environment Call From VS-Mode
                                         | 1 << 12  // Instruction Page Fault
                                         | 1 << 13  // Load Page Fault
                                         | 1 << 15; // Store Page Fault

  private:
    uintmax_t mtimecmp_;
    uintmax_t sip_;
    int core_;
    HypervisorContext *context_;
    VirtualMachine *vm_;
};

} // namespace DEPOS

#endif
