#pragma once

#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/PLIC.hpp>
#include <architecture/rv/ic/Exception.hpp>
#include <memory/Memory.hpp>

namespace rv {
class MIC {
    using Mode = MachineMode;
    using Context = ContextBase<Mode>;
    static constexpr bool ChangeStack = Meta::SAME<KernelMode, SupervisorMode>::Result;

    static void dispatch(Context *c) {
        uintmax_t mcause = csrr<Mode::CAUSE>();
        int code = (mcause << 1) >> 1;

        if (mcause & IC::INTERRUPT) {
            IC::dispatch(code);
        } else {
            if ((mcause == 8) | (mcause == 9) || (mcause == 10) | (mcause == 11)) {
                Syscall::dispatch(c->a0);
                c->pc += 4;
            } else {
                Exception<MachineMode>::dispatch();
            }
        }
    }

    __attribute__((naked, aligned(4))) static void entry() {
        dispatch(Context::push<ChangeStack>());
        Context::pop<ChangeStack>();
    }

  public:
    class Syscall {
      public:
        enum { TIME = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E' };

        static void dispatch(unsigned int id) {
            if (id == TIME) {
                CLINT::syscall();
            }
        }
    };

    static void init() {
        csrw<MachineMode::TVEC>(MIC::entry);

        if constexpr (ChangeStack) {
            csrw<Mode::SCRATCH>(reinterpret_cast<unsigned long>(Memory::alloc(4096)));
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
