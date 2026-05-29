#pragma once

#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {
class Core {
  public:
    static constexpr unsigned int EID = 0;

    static void handler(ContextFrame *c) {
        c->a0 = CPU::id();
        c->a1 = 0;
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
