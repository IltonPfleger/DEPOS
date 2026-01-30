#pragma once

#include <Macros.hpp>
// #include <machine/Traits.hpp>
#include <Traits.hpp>

class Application;
template <> struct Traits<Application> {
    static constexpr unsigned long Addr = Traits<MemoryMap>::RamStart + 0x200000;
    static constexpr unsigned long Linux = 0x80200000;
    static constexpr unsigned long DTB = 0x82200000;
    static constexpr unsigned long Initramfs = 0x88000000;
};
