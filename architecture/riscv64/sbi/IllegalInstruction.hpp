#pragma once

#include <Alarm.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/Decoder.hpp>
#include <architecture/riscv64/ExceptionHandler.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/Timer.hpp>

namespace QUARK ::sbi {

class IllegalInstruction {
  public:
    static constexpr unsigned int CODE = 2;

    static void dispatch(size_t id, ContextFrame *context) {
        uint32_t instruction = context->value & 0xFFFFFFFF;
        if (Decoder::rdtime(instruction)) {
            uint8_t rd     = Decoder::rd(instruction);
            (*context)[rd] = CLINT::mtime();
            context->pc += 4;
        } else if (Decoder::wfi(instruction)) {
            Alarm(0);
            context->pc += 4;
        } else if (Decoder::fp(instruction) && !FPU::enabled(context)) {
            FPU::enable<MachineMode>(context);
        } else {
            ExceptionHandler::onTrap(id, context);
        }
    }
};

} // namespace QUARK::sbi
