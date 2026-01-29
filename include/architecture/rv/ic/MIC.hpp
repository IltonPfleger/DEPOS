#pragma once

#include <Traits.hpp>
#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/PLIC.hpp>
#include <architecture/rv/ic/Exception.hpp>
#include <architecture/rv/ic/MachineSyscall.hpp>
#include <architecture/rv/ic/sbi/SBI.hpp>
#include <memory/Memory.hpp>

namespace rv {
class MIC {
  private:
    using Mode = MachineMode;
    using Syscall = Meta::TypeSelector<Traits<System>::Hypervisor, sbi::SBI, MachineSyscall>::Result;

    static constexpr bool ChangeStack = Traits<RISCV>::Supervisor;

    static void dispatch(MachineContext *c) {
        uintmax_t mcause = csrr<Mode::CAUSE>();
        int code = (mcause << 1) >> 1;

        if (mcause & IC::INTERRUPT) {
            IC::dispatch(code);
        } else {
            if (!Syscall::dispatch(c)) Exception<MachineMode>::dispatch();
        }
    }

    __attribute__((naked, aligned(4))) static void entry() {
        dispatch(MachineContext::push<ChangeStack>());
        MachineContext::pop<ChangeStack>();
    }

  public:
    static void init() {
        csrw<MachineMode::TVEC>(MIC::entry);

        if constexpr (ChangeStack) {
            csrw<Mode::SCRATCH>(reinterpret_cast<unsigned long>(Memory::alloc(4096)) + 4096);
        }

        if constexpr (CLINT::Enable) {
            CLINT::init();
            IC::bind(7, CLINT::handler);
        }

        if constexpr (PLIC::Enable) {
            PLIC::init();
            IC::bind(11, PLIC::handler);
        }
    }
};

} // namespace rv
