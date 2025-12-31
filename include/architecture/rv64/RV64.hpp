#pragma once

#include <Timer.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <architecture/common/Atomic.hpp>
#include <utils/Debug.hpp>

class RV64 {
  public:
    struct MachineMode;
    struct SupervisorMode;
    struct HypervisorMode;
    struct UserMode;
    using KernelMode = Meta::TypeSelector<Traits<System>::MULTITASK, SupervisorMode, MachineMode>::Result;

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
#include "CPU.hpp"
/**/
#include "IC.hpp"
/**/
#include "MMU.hpp"
};
