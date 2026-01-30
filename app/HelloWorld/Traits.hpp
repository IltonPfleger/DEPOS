#pragma once

#include <Macros.hpp>
#include <machine/Traits.hpp>

template <typename T> struct Traits;

template <> struct Traits<Application> {
    static constexpr unsigned long Addr = Traits<MemoryMap>::RamStart + 0x200000;
    static constexpr bool Virtualized = false;
};
