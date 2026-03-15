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

namespace sbi {

class SBI {
  public:
    static void init() {
        IC::bind(Syscall::CODE, Syscall::dispatch, false, false);
        IC::bind(IllegalInstruction::CODE, IllegalInstruction::dispatch, false, false);
        IC::bind(LoadAccessFault::CODE, LoadAccessFault::dispatch, false, false);
        IC::bind(StoreAccessFault::CODE, StoreAccessFault::dispatch, false, false);
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
