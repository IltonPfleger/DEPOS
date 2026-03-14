#pragma once

#include <Traits.hpp>

namespace DEPOS {

namespace riscv64 {
class CPU;
class CLINT;
class PLIC;
class IC;
} // namespace riscv64

class RISCV;

template <> struct Traits<RISCV> {
    static constexpr bool Supervisor = Traits<Kernel>::Multitask;
    static constexpr bool Hypervisor = Traits<Application>::Virtualized;

    static_assert(!(Supervisor && Hypervisor));
};

template <> struct Traits<riscv64::CPU> : Traits<CPU> {};
template <> struct Traits<riscv64::CLINT> : Traits<CLINT> {};
template <> struct Traits<riscv64::PLIC> : Traits<PLIC> {};

} // namespace DEPOS
