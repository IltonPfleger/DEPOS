#pragma once

#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/Context.hpp>

namespace rv {
class ReducedSBI {
  public:
    enum { TIME = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E' };

    static bool dispatch(MachineContext *c) {
        bool handle = false;
        uintmax_t mcause = csrr<MachineMode::CAUSE>();

        if (mcause == 9) {
            if (c->a0 == TIME) {
                CLINT::syscall();
                handle = true;
                c->pc += 4;
            }
        } else {
            Exception<SupervisorMode>::dispatch();
        }
        return handle;
    }
};
} // namespace rv
