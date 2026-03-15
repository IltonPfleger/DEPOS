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
    static void external(unsigned int) {
        unsigned int id = PLIC::claim();
        if (id) IC::dispatch(id, true, true);
        PLIC::complete(id);
    }

    static void dispatch(Context *) {
        //    intmax_t mcause = csrr<MachineMode::CAUSE>();
        //    CPU::gp(static_cast<Context *>(context));
        //    if (!(mcause >> 63)) {
        //        if (!sbi::SBI::dispatch(context)) {
        //            Exception::dispatch();
        //        }
        //    } else {
        //        bool interruption = mcause >> 63;
        //        bool external     = false;
        //        mcause &= ~(1ULL << 63);
        //        IC::dispatch(mcause, interruption, external);
        //        VirtualCPU::handler();
        //    }
    }

    // static void dispatch(MachineContext *) {
    // uintmax_t mcause = csrr<MachineMode::CAUSE>();

    // if (mcause & IC::INTERRUPT) {
    //     unsigned int id = mcause & ~IC::INTERRUPT;
    //     if (id == 11) {
    //         id = PLIC::claim();
    //         if (id) {
    //             IC::dispatch(id + 11);
    //         }
    //         PLIC::complete(id);
    //         return;
    //     }
    //     VirtualCPU::handler();
    //     IC::dispatch(id);
    //     return;
    // }

    // if (!sbi::SBI::dispatch(c)) {
    //     Exception<MachineMode>::dispatch();
    // }
    //}

    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        dispatch(MachineContext::push<true>());
        MachineContext::pop<true>();
    }

  public:
    static void init() {
        csrw<MachineMode::TVEC>(entry);

        IC::bind(7, [](unsigned int) { CLINT::write(); }, true, false);
        csrs<MachineMode::IE>(MachineMode::TI);

        PLIC::init();
        IC::bind(11, external, true, false);
        csrs<MachineMode::IE>(MachineMode::EI);
    }
};

} // namespace riscv64

} // namespace DEPOS
