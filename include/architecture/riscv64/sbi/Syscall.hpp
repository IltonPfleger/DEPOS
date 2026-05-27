#pragma once

#include <architecture/riscv64/sbi/Base.hpp>
#include <architecture/riscv64/sbi/Core.hpp>
#include <architecture/riscv64/sbi/Time.hpp>

namespace DEPOS::riscv64::sbi {

class Syscall {
  public:
    static constexpr unsigned int CODE = 9;
    static void dispatch(size_t id, ContextFrame *c) {
        switch (c->a7) {
            case Base::EID: Base::handler(c); break;
            case Time::EID: Time::handler(c); break;
            case Core::EID: Core::handler(c); break;
            default: ExceptionHandler::onTrap(id, c);
        }
        c->pc += 4;
    }
};

} // namespace DEPOS::riscv64::sbi
