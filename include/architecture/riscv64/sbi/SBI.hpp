#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <architecture/riscv64/sbi/IllegalInstruction.hpp>
#include <architecture/riscv64/sbi/LoadAccessFault.hpp>
#include <architecture/riscv64/sbi/StoreAccessFault.hpp>
#include <architecture/riscv64/sbi/Syscall.hpp>

namespace riscv64 {

namespace sbi {

class SBI {
  public:
    static bool dispatch(MachineContext *c) {
        uintmax_t mcause = csrr<MachineMode::CAUSE>();

        switch (mcause) {
        case Syscall::CODE:
            return sbi::Syscall::handler(c);
        case IllegalInstruction::CODE:
            return sbi::IllegalInstruction::handler(c);
        case LoadAccessFault::CODE:
            return sbi::LoadAccessFault::handler(c);
        case StoreAccessFault::CODE:
            return sbi::StoreAccessFault::handler(c);
        }

        return false;
    }
};

} // namespace sbi

} // namespace riscv64
