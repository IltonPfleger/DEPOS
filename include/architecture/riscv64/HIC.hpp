#pragma once

#include <Traits.hpp>
#include <architecture/riscv64/IC.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <architecture/riscv64/TrapHandler.hpp>
#include <architecture/riscv64/sbi/SBI.hpp>

namespace DEPOS {

namespace riscv64 {

class HIC {
  public:
    static void init() {
        TrapHandler::init<MachineMode, true>();
        SBI::init();
        PLIC::init();
        TrapHandler::install(11, IC::onTrap);
        csrs<MachineMode::IE>(MachineMode::EI);
    }
};

} // namespace riscv64

} // namespace DEPOS
