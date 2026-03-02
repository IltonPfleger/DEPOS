#pragma once

#include <Macros.hpp>
#include <Traits.hpp>

namespace DEPOS {

template <typename> class NIC;
class Virtual;
namespace virtio {
template <typename, unsigned long> class Console;
template <typename, unsigned long> class Network;
} // namespace virtio

namespace riscv64 {
class VirtualCPU;
}

class Application;
template <> struct Traits<Application> {
    static constexpr unsigned long Addr = Traits<MemoryMap>::RamStart + 128 * 1024;
    static constexpr bool Virtualized   = true;
};

template <> struct Traits<Virtual> {
    typedef Meta::TypeList<riscv64::VirtualCPU,
                           virtio::Console<UART16550<UART0>, 0x30000000>,
                           virtio::Network<NIC<DWC_Ether_QoS<GMAC0>>, 0x30200000>>
        Devices;

    // typedef Meta::TypeList<riscv64::VirtualCPU, virtio::Console<UART16550<UART0>, 0x30000000>>
    // Devices;
};

} // namespace DEPOS
