#pragma once

#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/Context.hpp>

namespace rv {
class MachineSyscall {
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
} // namespace rv
