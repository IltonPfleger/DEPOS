#pragma once

#include <Timer.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <utils/Debug.hpp>

class RV64 {
  public:
    class Machine;
    class Supervisor;
    class Hypervisor;
    class User;
    using KernelMode = Meta::TypeSelector<Traits<System>::MULTITASK, Supervisor, Machine>::Result;

#include "CSRS.hpp"
/**/
#include "CLINT.hpp"
/**/
#include "Modes.hpp"
/**/
#include "Context.hpp"
/**/
#include "modes/Machine.hpp"
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
};
