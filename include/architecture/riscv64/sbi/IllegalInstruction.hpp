#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {

class IllegalInstruction {
  public:
    static constexpr unsigned int CODE = 2;

    enum { RDTIME = 0xC0102073, RDTIME_MASK = 0xFFF0707F };

    static void dispatch(unsigned int id, Context *c) {
        uint32_t tval = static_cast<uint32_t>(csrr<MachineMode::TVAL>());

        if ((tval & RDTIME_MASK) == RDTIME) {
            unsigned int rd = (tval >> 7) & 0x1F;
            if (rd != 0) (*c)[rd] = CLINT::read();
            c->pc += 4;
        } else {
            Exception::dispatch(id, c);
        }
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
