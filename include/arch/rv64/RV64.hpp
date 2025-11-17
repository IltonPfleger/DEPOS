#pragma once

#include <Timer.hpp>
#include <Traits.hpp>
#include <Types.hpp>

class RISCV {
  public:
    struct MachineMode;
    struct SupervisorMode;
    struct HypervisorMode;
    struct UserMode;
    using KernelMode = MachineMode;

#include "CSRS.hpp"
/**/
#include "CLINT.hpp"
/**/
#include "Modes.hpp"
/**/
#include "Context.hpp"
/**/
#include "Interruptions.hpp"
/**/
#include "Atomic.hpp"
/**/
#include "CPU.hpp"
/**/
#include "IC.hpp"
/**/
#include "MMU.hpp"
}; // namespace RISCV
