#pragma once

#include <Traits.hpp>
#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/PLIC.hpp>
#include <architecture/rv/ic/Exception.hpp>
#include <architecture/rv/ic/ReducedSBI.hpp>
#include <architecture/rv/ic/sbi/SBI.hpp>
#include <memory/Memory.hpp>

namespace rv {
class MIC {
  private:
    using Mode = MachineMode;
    // using Syscall = Meta::IF<Traits<Application>::Virtualized, sbi::SBI, ReducedSBI>::Result;

    static constexpr bool ChangeStack = Traits<RISCV>::Supervisor;

    static void dispatch(MachineContext *) {
        uintmax_t mcause = csrr<Mode::CAUSE>();

        if (mcause & IC::INTERRUPT) {
            IC::dispatch(mcause & ~IC::INTERRUPT);
        } else {
            Exception<MachineMode>::dispatch();
            // if (!Syscall::dispatch(c)) Exception<MachineMode>::dispatch();
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

        // if constexpr (CLINT::Enable) {
        //     IC::bind(7, CLINT::handler);
        //     CLINT::init();
        // }

        // if constexpr (PLIC::Enable) {
        //     IC::bind(11, PLIC::handler);
        //     PLIC::init();
        // }
    }
};

} // namespace rv
