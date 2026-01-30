#pragma once

#include <Macros.hpp>
// #include <machine/Traits.hpp>
#include <Traits.hpp>

class Application;
template <> struct Traits<Application> {
    static constexpr unsigned long Addr = Traits<MemoryMap>::RamStart + 0x200000;
    static constexpr bool Virtualized = true;
};
