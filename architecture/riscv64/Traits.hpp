#pragma once

#include <Traits.hpp>

namespace QUARK {

class RISCV;

template <> struct Traits<RISCV> {
    static constexpr bool Supervisor = Traits<Kernel>::Multitask;
    static constexpr bool Hypervisor = Traits<Payload>::Virtualized;
    static_assert(!(Supervisor && Hypervisor));
};

} // namespace QUARK
