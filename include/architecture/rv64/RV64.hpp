#pragma once

#include <Timer.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <utils/Debug.hpp>
#include <utils/DispatchTable.hpp>

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
#include "Context.hpp"
/**/
#include "Atomic.hpp"
/**/
#include "Interruptions.hpp"
/**/
#include "CPU.hpp"
/**/
#include "modes/Machine.hpp"
/**/
#include "modes/Supervisor.hpp"
/**/
#include "MMU.hpp"
};
