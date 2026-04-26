#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/PMP.hpp>
#include <architecture/riscv64/csrs.hpp>
#include <architecture/riscv64/sbi/IllegalInstruction.hpp>
#include <architecture/riscv64/sbi/LoadAccessFault.hpp>
#include <architecture/riscv64/sbi/StoreAccessFault.hpp>
#include <architecture/riscv64/sbi/Syscall.hpp>

namespace DEPOS {

namespace riscv64 {

class SBI {
  public:
    static void init() {
        TrapHandler::install(sbi::Syscall::CODE, sbi::Syscall::dispatch, TrapHandler::Exception);
        TrapHandler::install(sbi::IllegalInstruction::CODE, sbi::IllegalInstruction::dispatch, TrapHandler::Exception);
        TrapHandler::install(sbi::LoadAccessFault::CODE, sbi::LoadAccessFault::dispatch, TrapHandler::Exception);
        TrapHandler::install(sbi::StoreAccessFault::CODE, sbi::StoreAccessFault::dispatch, TrapHandler::Exception);
    }
};

} // namespace riscv64

} // namespace DEPOS
