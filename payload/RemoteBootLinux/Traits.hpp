#pragma once

#include <Traits.hpp>

namespace QUARK {

// class Virtual;
// namespace virtio {
// template <typename, unsigned long> class Console;
// template <typename, unsigned long> class Network;
// } // namespace virtio
//
// namespace riscv64 {
// class VirtualCPU;
// }
//
// namespace hypervisor {
// template <typename> class VirtualSwitch;
// }

class Payload;
template <> struct Traits<Payload> {
    static constexpr unsigned long Address = Traits<MemoryMap>::RamStart + 128 * 1024;
    static constexpr bool Virtualized   = true;
};

// template <> struct Traits<Virtual> {
//  typedef Meta::TypeList<virtio::Console<UART16550<UART0>, 0x30000000>,
//                         virtio::Network<hypervisor::VirtualSwitch<DWC_Ether_QoS<GMAC0>>, 0x30200000>,
//                         // virtio::Network<DWC_Ether_QoS<GMAC0>, 0x30200000>,
//                         riscv64::VirtualCPU>
//      Devices;

// typedef Meta::TypeList<riscv64::VirtualCPU, virtio::Console<UART16550<UART0>, 0x30000000>>
// Devices;
//};

} // namespace QUARK
