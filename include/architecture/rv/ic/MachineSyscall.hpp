#pragma once

#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/Context.hpp>

namespace rv {
class MachineSyscall {
  public:
    enum { TIME = 0 };

    static bool dispatch(MachineContext *c) {
        bool handle = false;
        uintmax_t mcause = csrr<MachineMode::CAUSE>();

        TraceIn();
        if (mcause == 9) {
            if (c->a0 == TIME) {
                CLINT::syscall();
                handle = true;
            }
            c->pc += 4;
        }
        return handle;
    }
};
} // namespace rv
