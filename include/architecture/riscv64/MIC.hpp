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

    static void external(unsigned int) {
        unsigned int id = PLIC::claim();
        if (id) IC::dispatch(id, true, true);
        PLIC::complete(id);
    }

    static void dispatch(MachineContext *context) {
        intmax_t mcause = csrr<MachineMode::CAUSE>();
        CPU::gp(static_cast<Context *>(context));
        bool interruption = mcause >> 63;
        bool external     = false;
        mcause &= ~(1ULL << 63);
        IC::dispatch(mcause, interruption, external);
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
            IC::bind(7, CLINT::forward, true, false);
            csrs<MachineMode::IP>(SupervisorMode::TI);
        }

        if constexpr (!Traits<RISCV>::Supervisor && Traits<PLIC>::Enable) {
            PLIC::init();
            IC::bind(11, external, true, false);
            csrs<MachineMode::IE>(MachineMode::EI);
        }
    }
};

} // namespace riscv64

} // namespace DEPOS
