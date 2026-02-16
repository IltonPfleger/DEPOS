#pragma once

#include <Traits.hpp>

class RISCV;

template <> struct Traits<RISCV> {
    static constexpr bool Supervisor = Traits<System>::Multitask;
    static constexpr bool Hypervisor = Traits<Application>::Virtualized;
};
