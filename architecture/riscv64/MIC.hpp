#ifndef __RISCV64_MIC_HEADER
#define __RISCV64_MIC_HEADER

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Decoder.hpp>
#include <architecture/riscv64/FPU.hpp>
#include <architecture/riscv64/IC.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <architecture/riscv64/TrapHandler.hpp>

namespace QUARK {

class MIC {
    static constexpr bool IsMachineMode                 = !Traits<RISCV>::Supervisor;
    static constexpr bool IsTimerEnable                 = Traits<QUARK::Timer>::Enable;
    static constexpr bool IsExternalInterruptionsEnable = Traits<PLIC>::Enable;
    static constexpr bool ChangeStack = (IsMachineMode && Traits<Thread>::IsolatedKernelStack) || !IsMachineMode;
    using MachineContextHandler       = MachineContext<ChangeStack>;

  protected:
    static void forward(size_t, ContextFrame *) { CLINT::forward(); }

    static void syscall(size_t, ContextFrame *context) {
        CLINT::syscall();
        context->pc += 4;
    }

    static void fpu(size_t, ContextFrame *context) {
        if (Decoder::fp(context->value)) {
            FPU::enable<MachineMode>(context);
        }
    }

  public:
    static void init() {
        TrapHandler::init<MachineMode, ChangeStack>();

        if constexpr (!IsMachineMode) {
            CoreContextHandler<MachineMode>::stack(__amm.end() - Traits<Memory>::PageSize * CPU::id<true>());

            if constexpr (IsTimerEnable) {
                csrs<MachineMode::IP>(SupervisorMode::TI);
                TrapHandler::install(7, forward);
                TrapHandler::install(9, syscall, TrapHandler::Exception);
            }

            return;
        }

        TrapHandler::install(2, fpu, TrapHandler::Exception);

        if constexpr (IsExternalInterruptionsEnable) {
            PLIC::init();
            TrapHandler::install(11, IC::onTrap);
            csrs<MachineMode::IE>(MachineMode::EI);
        }
    }
};

} // namespace QUARK

#endif
