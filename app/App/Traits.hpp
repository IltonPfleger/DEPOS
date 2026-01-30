#pragma once

#include <machine/Traits.hpp>

class Application;

template <> struct Traits<Application> {
    static constexpr unsigned long Addr = Traits<MemoryMap>::RamStart + 0x00200000;
};
