#pragma once

#include <architecture/riscv64/ContextFrame.hpp>

namespace DEPOS::riscv64::sbi {

class FWFT {
  public:
    static constexpr unsigned int EID = 0x46574654;
    static void handler(ContextFrame *c) {
        switch (c->a6) {
            default: Console::println(Console::Hex(c->a6));
        }
    }
};

} // namespace DEPOS::riscv64::sbi
