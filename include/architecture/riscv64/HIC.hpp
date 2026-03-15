#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Exception.hpp>
#include <architecture/riscv64/MIC.hpp>
#include <architecture/riscv64/Timer.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>
#include <architecture/riscv64/sbi/SBI.hpp>
#include <memory/Memory.hpp>
#include <utils/Debug.hpp>

namespace DEPOS {

namespace riscv64 {

class HIC : MIC {

  private:
    static void dispatch(Context *context) {
        MIC::dispatch(context);
        VirtualCPU::handler();
    }

    __attribute__((naked, optimize("O0"), aligned(4))) static void entry() {
        dispatch(MachineContext::push<true>());
        MachineContext::pop<true>();
    }

  public:
    static void init() {
        static_assert(!Traits<Application>::Virtualized || Traits<Thread>::IsolatedKernelStack);

        csrw<MachineMode::TVEC>(entry);

        for (int i = 0; i < 16; i++)
            IC::bind(i, Exception::dispatch, false, false);

        sbi::SBI::init();

        PLIC::init();
        IC::bind(0, +[](unsigned int) {}, true, true);
        IC::bind(11, MIC::external, true, false);
        csrs<MachineMode::IE>(MachineMode::EI);
    }
};

} // namespace riscv64

} // namespace DEPOS
