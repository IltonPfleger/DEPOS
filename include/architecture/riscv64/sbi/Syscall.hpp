#pragma once

#include <architecture/riscv64/sbi/Base.hpp>
#include <architecture/riscv64/sbi/Time.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {

class Syscall {
  public:
    static constexpr unsigned int CODE = 9;

    static bool handler(MachineContext *c) {
        switch (c->a7) {
        case Base::EID:
            c->pc += 4;
            return Base::handler(c);
        case Time::EID:
            c->pc += 4;
            return Time::handler(c);
        }
        return false;
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
