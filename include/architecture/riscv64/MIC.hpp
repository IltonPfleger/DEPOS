#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Exception.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <memory/Memory.hpp>

namespace riscv64 {
class MIC {
  private:
    static constexpr bool ChangeStack = Traits<RISCV>::Supervisor || Traits<RISCV>::Hypervisor;

    static void dispatch(MachineContext *c) {
        uintmax_t mcause = csrr<MachineMode::CAUSE>();

        if (mcause & IC::INTERRUPT) {
            unsigned int id = mcause & ~IC::INTERRUPT;

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

        if constexpr (ChangeStack) {
            char *stack = reinterpret_cast<char *>(Memory::alloc(Traits<Memory>::StackSize)) + Traits<Memory>::StackSize;
            csrw<MachineMode::SCRATCH>(stack);
        }

        if constexpr (Traits<::Timer>::Enable && Traits<RISCV>::Supervisor) {
            IC::bind(7, CLINT::forward);
            csrs<MachineMode::IE>(MachineMode::TI);
            CLINT::write();
        }

        if constexpr (!Traits<RISCV>::Supervisor && Traits<::PLIC>::Enable) {
            PLIC::init();
            csrs<MachineMode::IE>(MachineMode::EI);
        }
    }
};

} // namespace riscv64
