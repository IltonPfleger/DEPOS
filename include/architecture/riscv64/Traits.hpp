#pragma once

#include <Traits.hpp>

class RISCV;

template <> struct Traits<RISCV> {
    static constexpr bool Supervisor = true; // Traits<System>::Multitask || Traits<Application>::Virtualized;
};
