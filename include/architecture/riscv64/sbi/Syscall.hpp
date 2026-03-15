#pragma once

#include <architecture/riscv64/sbi/Base.hpp>
#include <architecture/riscv64/sbi/Time.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {

class Syscall {
  public:
    static constexpr unsigned int CODE = 9;

    static void dispatch(unsigned int id, Context *c) {
        switch (c->a7) {
        case Base::EID:
            Base::handler(c);
            break;
        case Time::EID:
            Time::handler(c);
            break;
        default:
            Exception::dispatch(id, c);
        }
        c->pc += 4;
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
