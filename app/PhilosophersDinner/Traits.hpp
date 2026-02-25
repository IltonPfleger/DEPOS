#pragma once

#include <Traits.hpp>

namespace DEPOS {

template <> struct Traits<Application> {
    static constexpr unsigned long Addr = Traits<MemoryMap>::RamStart + 128 * 1024;
    static constexpr bool Virtualized = false;
};

} // namespace DEPOS
