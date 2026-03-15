#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Exception.hpp>
#include <architecture/riscv64/IC.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <memory/Memory.hpp>

namespace DEPOS {

namespace riscv64 {

class MIC {
    static constexpr bool ChangeStack = Traits<Thread>::IsolatedKernelStack || Traits<Kernel>::Multitask;

  protected:
    static void external(unsigned int) {
        unsigned int id = PLIC::claim();
        IC::dispatch(id, 0, true, true);
        PLIC::complete(id);
    }

    static void dispatch(Context *context) {
        intmax_t mcause   = csrr<MachineMode::CAUSE>();
        bool interruption = mcause >> 63;
        bool external     = false;
        mcause &= ~(1ULL << 63);
        IC::dispatch(mcause, context, interruption, external);
    }

    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        dispatch(MachineContext::push<ChangeStack>());
        MachineContext::pop<ChangeStack>();
    }

  public:
    static void init() {
        csrw<MachineMode::TVEC>(MIC::entry);

        for (int i = 0; i < 16; i++)
            IC::bind(i, Exception::dispatch, false, false);

        if constexpr (Traits<Kernel>::Multitask) {
            /* Keep Boot Stack For Handle M-Mode IRQs */
            csrw<MachineMode::SCRATCH>(Traits<MemoryMap>::PhysicalRamEnd - Traits<Memory>::PageSize * CPU::id());
        }

        if constexpr (Traits<DEPOS::Timer>::Enable && Traits<RISCV>::Supervisor) {
            IC::bind(7, CLINT::forward, true, false);
            csrs<MachineMode::IP>(SupervisorMode::TI);
        }

        if constexpr (!Traits<RISCV>::Supervisor && Traits<PLIC>::Enable) {
            PLIC::init();
            IC::bind(0, +[](unsigned int) {}, true, true);
            IC::bind(11, external, true, false);
            csrs<MachineMode::IE>(MachineMode::EI);
        }
    }
};

} // namespace riscv64

} // namespace DEPOS
