#pragma once

#include <Traits.hpp>

class RISCV;

template <> struct Traits<RISCV> {
    static constexpr bool Supervisor = Traits<Kernel>::Multitask;
    static constexpr bool Hypervisor = Traits<Application>::Virtualized;

    static_assert(!(Supervisor && Hypervisor));
};
