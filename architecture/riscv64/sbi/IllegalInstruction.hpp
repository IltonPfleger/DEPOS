#pragma once

#include <Alarm.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/Decoder.hpp>
#include <architecture/riscv64/ExceptionHandler.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/Timer.hpp>

namespace DEPOS ::sbi {

class IllegalInstruction {
  public:
    static constexpr unsigned int CODE = 2;

    static void dispatch(size_t id, ContextFrame *c) {
        uint32_t instruction = static_cast<uint32_t>(csrr<MachineMode::TVAL>());
        if (Decoder::rdtime(instruction)) {
            uint8_t rd = Decoder::rd(instruction);
            (*c)[rd]   = CLINT::mtime();
            c->pc += 4;
        } else if (Decoder::wfi(instruction)) {
            Alarm(0);
            c->pc += 4;
        } else if (Decoder::fp(instruction)) {
            c->status &= ~(3ULL << 13);
            c->status |= (1ULL << 13);
        } else {
            ExceptionHandler::onTrap(id, c);
        }
    }
};

} // namespace DEPOS::sbi
