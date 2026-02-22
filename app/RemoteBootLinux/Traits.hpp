#pragma once

#include <Macros.hpp>
#include <Traits.hpp>

class Application;
template <> struct Traits<Application> {
    static constexpr unsigned long Addr = Traits<MemoryMap>::RamStart + 128 * 1024;
    static constexpr bool Virtualized = true;
};

class Virtual;

namespace riscv64 {
class VirtualCPU;
}

namespace virtio {
template <unsigned long> class Console;
}

template <> struct Traits<Virtual> {
    typedef Meta::TypeList<riscv64::VirtualCPU, virtio::Console<0x30000000>> Devices;
};
