#pragma once

#include <Traits.hpp>
#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/PLIC.hpp>
#include <architecture/rv/ic/Exception.hpp>
#include <architecture/rv/ic/sbi/SBI.hpp>
#include <memory/Memory.hpp>

namespace rv {
class MIC {
  public:
    class SupervisorSyscall {
      public:
        enum { TIME = 0 };

        static bool dispatch(MachineContext *c) {
            bool handle = true;
            if (c->a0 == TIME) {
                CLINT::syscall();
            } else {
                handle = false;
            }
            c->pc += 4;
            return handle;
        }
    };

  private:
    using Mode = MachineMode;
    using Syscall = Meta::TypeSelector<Traits<System>::Hypervisor, sbi::SBI, SupervisorSyscall>::Result;

    static constexpr bool ChangeStack = Meta::SAME<KernelMode, SupervisorMode>::Result;

    static void dispatch(MachineContext *c) {
        uintmax_t mcause = csrr<Mode::CAUSE>();
        int code = (mcause << 1) >> 1;

        if (mcause & IC::INTERRUPT) {
            IC::dispatch(code);
        } else {
            if (!Syscall::dispatch(c)) Exception<MachineMode>::dispatch();
        }
    }

    __attribute__((naked, aligned(4))) static void entry() {
        dispatch(MachineContext::push<ChangeStack>());
        MachineContext::pop<ChangeStack>();
    }

  public:
    static void init() {
        csrw<MachineMode::TVEC>(MIC::entry);

        if constexpr (ChangeStack) {
            csrw<Mode::SCRATCH>(reinterpret_cast<unsigned long>(Memory::alloc(4096)) + 4096);
        }

        if constexpr (CLINT::Enable) {
            CLINT::init();
            IC::bind(7, CLINT::handler);
        }

        if constexpr (PLIC::Enable) {
            PLIC::init();
            IC::bind(11, PLIC::handler);
        }
    }
};

} // namespace rv
