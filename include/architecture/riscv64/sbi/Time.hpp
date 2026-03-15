#pragma once

#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {
class Time {
  public:
    static constexpr unsigned int EID = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E';

    static bool handler(Context *c) {
        VirtualCPU::reset(c->a0);
        c->a0 = 0;
        c->a1 = 0;
        return true;
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
