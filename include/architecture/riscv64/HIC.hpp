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
    static_assert(Traits<Thread>::IsolatedKernelStack);

  private:
    static void dispatch(MachineContext *c) {
        uintmax_t mcause = csrr<MachineMode::CAUSE>();

        if (mcause & IC::INTERRUPT) {
            unsigned int id = mcause & ~IC::INTERRUPT;
            if (id == 11) {
                id = PLIC::claim();
                if (id) {
                    IC::dispatch(id + 11);
                }
                PLIC::complete(id);
                return;
            }
            VirtualCPU::handler();
            IC::dispatch(id);
            return;
        }

        if (!sbi::SBI::dispatch(c)) {
            Exception<MachineMode>::dispatch();
        }
    }

    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        dispatch(MachineContext::push<true>());
        MachineContext::pop<true>();
    }

  public:
    static void init() {
        csrw<MachineMode::TVEC>(entry);

        IC::bind(7, [](unsigned int) { CLINT::write(); });
        csrs<MachineMode::IE>(MachineMode::TI);

        PLIC::init();
        csrs<MachineMode::IE>(MachineMode::EI);
    }
};

} // namespace riscv64

} // namespace DEPOS
