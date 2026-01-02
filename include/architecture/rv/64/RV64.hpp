#pragma once

#include <Timer.hpp>
#include <Traits.hpp>
#include <Types.hpp>
#include <architecture/common/Atomic.hpp>
#include <architecture/rv/64/Context.hpp>
#include <architecture/rv/64/Modes.hpp>
#include <architecture/rv/csrs.hpp>
#include <utils/Debug.hpp>

using rv::csrc;
using rv::csrr;
using rv::csrs;
using rv::csrw;

template <typename TimerDevice> class RV64 {
  public:
    using MachineMode = rv::MachineMode;
    using SupervisorMode = rv::SupervisorMode;
    using KernelMode = Meta::TypeSelector<Traits<System>::MULTITASK, SupervisorMode, MachineMode>::Result;
    template <typename T> using ContextBase = rv64::ContextBase<T>;

#include "Interruptions.hpp"
/**/
#include "CPU.hpp"
/**/
#include "IC.hpp"
/**/
#include "MMU.hpp"
};
