#pragma once

#include <Traits.hpp>

namespace DEPOS {

class RISCV;
class FPU;

template <> struct Traits<FPU> {
    static constexpr bool Enabled = true;
};

template <> struct Traits<RISCV> {
    static constexpr bool Supervisor = Traits<Kernel>::Multitask;
    static constexpr bool Hypervisor = Traits<Application>::Virtualized;
    static_assert(!(Supervisor && Hypervisor));
};

} // namespace DEPOS
