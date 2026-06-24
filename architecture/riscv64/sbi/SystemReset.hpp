#pragma once

#include <Thread.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace QUARK::sbi {

class SystemReset {
  public:
    static constexpr unsigned int EID = 'S' << 24 | 'R' << 16 | 'S' << 8 | 'T';

    static void handler(ContextFrame *c) {
        if (c->a6 == 0) {
            Thread::exit();
        }
    }
};

} // namespace QUARK::sbi
