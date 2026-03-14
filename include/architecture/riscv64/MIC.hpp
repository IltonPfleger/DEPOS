#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Exception.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <memory/Memory.hpp>

namespace DEPOS {

namespace riscv64 {

class MIC {
  private:
    static constexpr bool ChangeStack = Traits<Thread>::IsolatedKernelStack || Traits<Kernel>::Multitask;

    static void dispatch(MachineContext *c) {
        intmax_t mcause = csrr<MachineMode::CAUSE>();
        uint32_t id     = mcause & ~IC::INTERRUPT;

        if (mcause < 0) {
            if (id == 11) {
                id = PLIC::claim();
                if (id) IC::dispatch(id + 11);
                PLIC::complete(id);
            } else {
                IC::dispatch(id);
            }
        } else {
            if (mcause == 9) {
                CLINT::syscall();
                c->pc += 4;
            } else
                Exception<MachineMode>::dispatch();
        }
    }

    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        dispatch(MachineContext::push<ChangeStack>());
        MachineContext::pop<ChangeStack>();
    }

  public:
    static void init() {
        csrw<MachineMode::TVEC>(MIC::entry);

        if constexpr (Traits<Kernel>::Multitask) {
            /* Keep Boot Stack For Handle M-Mode IRQs */
            csrw<MachineMode::SCRATCH>(Traits<MemoryMap>::PhysicalRamEnd - Traits<Memory>::PageSize * CPU::id());
        }

        if constexpr (Traits<DEPOS::Timer>::Enable && Traits<RISCV>::Supervisor) {
            IC::bind(7, CLINT::forward);
            csrs<MachineMode::IP>(SupervisorMode::TI);
        }

        if constexpr (!Traits<RISCV>::Supervisor && Traits<PLIC>::Enable) {
            PLIC::init();
            csrs<MachineMode::IE>(MachineMode::EI);
        }
    }
};

} // namespace riscv64

} // namespace DEPOS
