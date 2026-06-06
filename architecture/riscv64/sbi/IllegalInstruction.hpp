#pragma once

#include <Alarm.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/ExceptionHandler.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/Timer.hpp>
#include <architecture/riscv64/sbi/Decoder.hpp>

namespace DEPOS {

namespace sbi {

class IllegalInstruction {

  public:
    static constexpr unsigned int CODE = 2;

    enum { RDTIME = 0xC0102073, RDTIME_MASK = 0xFFF0707F };

    static void dispatch(size_t id, ContextFrame *c) {
        uint32_t tval = static_cast<uint32_t>(csrr<MachineMode::TVAL>());
        if (Decoder::fp(tval)) {
            c->status &= ~(3ULL << 13);
            c->status |= (1ULL << 13);
        } else if (Decoder::wfi(tval)) {
            Alarm(0);
            c->pc += 4;
        } else if (Decoder::rdtime(tval)) {
            uint8_t rd = Decoder::rd(tval);
            (*c)[rd]   = CLINT::mtime();
            c->pc += 4;
        } else {
            ExceptionHandler::onTrap(id, c);
        }
    }
};

} // namespace sbi

} // namespace DEPOS
