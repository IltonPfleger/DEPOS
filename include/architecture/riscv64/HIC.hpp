#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Exception.hpp>
#include <architecture/riscv64/Timer.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>
#include <architecture/riscv64/sbi/SBI.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

namespace DEPOS {

namespace riscv64 {

class HIC {
  private:
    using Mode = MachineMode;

    static void dispatch(MachineContext *c) {
        uintmax_t mcause = csrr<Mode::CAUSE>();

        if (mcause & IC::INTERRUPT) {
            unsigned int id = mcause & ~IC::INTERRUPT;

            if (id == 11) {
                id = PLIC::claim();
                if (id) {
                    IC::dispatch(id + 11);
                }
                PLIC::complete(id);
            } else {
                VirtualCPU::handler();
                IC::dispatch(id);
            }
        } else {
            if (!sbi::SBI::dispatch(c)) {
                Exception<MachineMode>::dispatch();
            }
        }
    }

    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        dispatch(MachineContext::push<true>());
        MachineContext::pop<true>();
    }

  public:
    static void init() {
        TraceIn();

        csrw<MachineMode::TVEC>(entry);

        char *stack = reinterpret_cast<char *>(Memory::alloc(Traits<Memory>::StackSize)) + Traits<Memory>::StackSize;
        csrw<Mode::SCRATCH>(stack);

        IC::bind(7, [](unsigned int) { CLINT::write(); });
        csrs<MachineMode::IE>(MachineMode::TI);
        // csrs<MachineMode::MCOUNTEREN>(~0);

        if constexpr (Traits<PLIC>::Enable) {
            PLIC::init();
            csrs<MachineMode::IE>(MachineMode::EI);
        }

        TraceOut();
    }
};

} // namespace riscv64

} // namespace DEPOS
