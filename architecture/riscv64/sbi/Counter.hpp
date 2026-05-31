#pragma once

#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS::sbi {

class Counter {
  public:
    static constexpr unsigned int EID = 0;

    static void handler(ContextFrame *c) {
        static size_t counter = 0;
        c->a0                 = CPU::Atomic::finc(counter);
        c->a1                 = 0;
    }
};

} // namespace DEPOS::sbi
