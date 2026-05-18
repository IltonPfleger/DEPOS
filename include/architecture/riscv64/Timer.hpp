#pragma once

#include <Traits.hpp>
#include <architecture/common/Timer.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/TrapHandler.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS {

namespace riscv64 {

class Timer : public ArchitectureCommon::Timer {

  public:
    static void dispatch(size_t, Context *) {
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

    static inline uint64_t us() { return CLINT::mtime() * 1'000'000 / Traits<CLINT>::Clock; }
    static inline uint64_t now() { return us(); }

    static void uspin(uintmax_t microseconds) {
        uintmax_t done = us() + microseconds;
        while (done > us())
            ;
    }

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
};

} // namespace riscv64

} // namespace DEPOS
