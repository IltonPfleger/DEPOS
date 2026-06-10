#ifndef __DEPOS_RISCV64_VCPU__
#define __DEPOS_RISCV64_VCPU__

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/MMU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <hypervisor/VirtualMachine.hpp>
#include <utility/Atomic.hpp>
#include <utility/Debug.hpp>

namespace DEPOS {

class VirtualCPU {
    struct Registers {
        uint64_t mtimecmp     = 0;
        uint64_t sscratch     = 0;
        uint64_t satp         = 0;
        uint64_t stvec        = 0;
        uint64_t scause       = 0;
        uint64_t stval        = 0;
        uint64_t sepc         = 0;
        uint64_t sie          = 0;
        Atomic<uintmax_t> sip = 0;
    };

  public:
    VirtualCPU(VirtualMachine *vm)
        : core_(-1),
          vm_(vm) {}

    void boot(auto... args) {
        CPU::Interrupt::disable();
        activate();
        csrs<MachineMode::STATUS>(MachineMode::TW | MachineMode::PP_S | MachineMode::PIRQE);
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

        restore();
    }

    void setInterruptPending() {
        registers_.sip |= SupervisorMode::EI;
        if (current() == this) {
            setExternalInterruptPending();
        } else if (core_ > 0) {
            CLINT::ipi(core_);
        }
    }

    void clearInterruptPending() {
        registers_.sip |= SupervisorMode::EI;
        if (current() == this) {
            clearExternalInterruptPending();
        } else if (core_ > 0) {
            CLINT::ipi(core_);
        }
    }

    static void onInterProcessorInterrupt() {
        if (!current()) return;
        csrs<MachineMode::IP>(current()->registers_.sip.load());
    }

    static void onTick() {
        if (!current()) return;
        if (CLINT::mtime() >= current()->registers_.mtimecmp) {
            setTimerInterruptPending();
        }
    }

    static uintmax_t mtimecmp() {
        assert(current());
        return current()->registers_.mtimecmp;
    }

    static void mtimecmp(uintmax_t mtimecmp) {
        assert(current());
        current()->registers_.mtimecmp = mtimecmp;
        if (mtimecmp <= CLINT::mtime()) {
            setTimerInterruptPending();
        } else {
            clearTimerInterruptPending();
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

    static VirtualCPU *current() { return current_[CPU::id()]; }

    void save() {
        core_ = -1;
        current(nullptr);
        registers_.sscratch = csrr<SupervisorMode::SCRATCH>();
        registers_.satp     = csrr<SupervisorMode::SATP>();
        registers_.stvec    = csrr<SupervisorMode::TVEC>();
        registers_.scause   = csrr<SupervisorMode::CAUSE>();
        registers_.stval    = csrr<SupervisorMode::TVAL>();
        registers_.sepc     = csrr<SupervisorMode::EPC>();
        registers_.sie      = csrr<MachineMode::IE>();
        registers_.sip |= csrr<MachineMode::IP>();
    }

  private:
    __attribute__((naked)) static void dispatch(auto... args) {
        ((void)args, ...);
        asm("mret");
    }

    void restore() {
        csrw<SupervisorMode::SCRATCH>(registers_.sscratch);
        csrw<SupervisorMode::SATP>(registers_.satp);
        csrw<SupervisorMode::TVEC>(registers_.stvec);
        csrw<SupervisorMode::CAUSE>(registers_.scause);
        csrw<SupervisorMode::TVAL>(registers_.stval);
        csrw<SupervisorMode::EPC>(registers_.sepc);
        csrc<MachineMode::IE>(0x222);
        csrs<MachineMode::IE>(registers_.sie & 0x222);
        csrc<MachineMode::IP>(0x222);
        csrs<MachineMode::IP>(registers_.sip & 0x222);
    }

    static void setTimerInterruptPending() { csrs<MachineMode::IP>(SupervisorMode::TI); }

    static void clearTimerInterruptPending() { csrc<MachineMode::IP>(SupervisorMode::TI); }

    static void setExternalInterruptPending() { csrs<MachineMode::IP>(SupervisorMode::EI); }

    static void clearExternalInterruptPending() { csrc<MachineMode::IP>(SupervisorMode::EI); }

    static void current(VirtualCPU *current) { current_[CPU::id()] = current; }

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
    int core_;
    Registers registers_;
    VirtualMachine *vm_;
};

} // namespace DEPOS

#endif
