#pragma once

#include <architecture/rv/Modes.hpp>
#include <architecture/rv/csrs.hpp>

namespace rv {
class Interruptions {
  public:
    static void disable() { csrc<KernelMode::STATUS>(KernelMode::IRQE); }
    static void enable() { csrs<KernelMode::STATUS>(KernelMode::IRQE); }

    static void pop() {
        if (--m_levels[CPU::id()] == 0) enable();
    }

    static bool push() {
        disable();
        m_levels[CPU::id()]++;
    }

  private:
    unsigned char m_levels[Traits<CPUS>::ACTIVE];
};
} // namespace rv
