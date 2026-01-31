#pragma once

#include <Traits.hpp>

template <typename T> struct Traits;

template <> struct Traits<Application> {
    static constexpr unsigned long Addr = Traits<MemoryMap>::RamStart + 128 * 1024;
    static constexpr bool Virtualized = false;
};
