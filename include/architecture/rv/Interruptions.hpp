#pragma once

#include <architecture/rv/Modes.hpp>
#include <architecture/rv/csrs.hpp>

namespace rv {
class Interruptions {
  public:
    static void disable() { csrc<KernelMode::STATUS>(KernelMode::IRQE); }
    static void enable() { csrs<KernelMode::STATUS>(KernelMode::IRQE); }

    static void on() { enable(); }
    static bool off() {
        unsigned long status = csrr<KernelMode::STATUS>();
        disable();
        return (status & KernelMode::IRQE) != 0;
    }
};
} // namespace rv
