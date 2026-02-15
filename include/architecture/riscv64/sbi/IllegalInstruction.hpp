#pragma once

#include <architecture/rv/Context.hpp>
#include <architecture/rv/Modes.hpp>

namespace rv {
namespace sbi {
class IllegalInstruction {
  public:
    static constexpr unsigned int CODE = 2;

    enum Opcode {
        SYSTEM = 0b1110011,
    };

    enum Function {
        CSRR = 0x2000,
    };

    enum {
        RDTIME = 11000000000100000,
    };

    static bool handler(MachineContext *c) {
        bool handle = false;
        uintmax_t tval = csrr<MachineMode::TVAL>();

        if (tval & SYSTEM) {
            if (tval & CSRR) {
                if (tval & RDTIME) {
                    unsigned int i = (tval >> 7) & 0x1f;
                    c->pc += 4;
                    (*c)[i] = CLINT::read();
                    handle = true;
                }
            }
        }
        return handle;
    }
};
} // namespace sbi
} // namespace rv
