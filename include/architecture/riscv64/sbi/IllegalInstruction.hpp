#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {

class IllegalInstruction {
  public:
    static constexpr unsigned int CODE = 2;

    enum Opcode {
        SYSTEM = 0x73,
    };

    enum Mask {
        OPCODE_MASK = 0x7F,
        FUNCT3_MASK = 0x7000,
        RD_MASK = 0xF80,
    };

    static constexpr uint32_t RDTIME_PATTERN = 0xC0102073;
    static constexpr uint32_t RDTIME_MASK = 0xFFF0707F;

    static bool handler(MachineContext *c) {
        uint32_t tval = static_cast<uint32_t>(csrr<MachineMode::TVAL>());

        if ((tval & RDTIME_MASK) == RDTIME_PATTERN) {
            unsigned int rd = (tval >> 7) & 0x1F;

            if (rd != 0) {
                (*c)[rd] = CLINT::read();
            }

            c->pc += 4;
            return true;
        }

        return false;
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
