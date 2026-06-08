#pragma once

#include <Traits.hpp>
#include <architecture/common/Timer.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/TrapHandler.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS {

class Timer : public ArchitectureCommon::Timer {
  public:
    static inline Microsecond us(uintmax_t ticks) { return Microsecond((ticks * 1'000'000) / Traits<CLINT>::Clock); }
    static inline Microsecond now() { return us(CLINT::mtime()); }

    static void init() {
        if constexpr (!Traits<RISCV>::Supervisor) {
            TrapHandler::install(7, dispatch);
            csrs<MachineMode::IE>(MachineMode::TI);
            CLINT::write();
        } else {
            TrapHandler::install(5, dispatch);
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
        }
    }

  private:
    static void dispatch(size_t, ContextFrame *) {
        if constexpr (Traits<Application>::Virtualized) {
            CLINT::write();
            VirtualCPU::onTick();
        } else if (!Traits<RISCV>::Supervisor) {
            CLINT::write();
        } else {
            CPU::syscall();
        }
        ArchitectureCommon::Timer::onTick(CPU::id());
    }
};

} // namespace DEPOS
