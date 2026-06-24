#pragma once

#include <Traits.hpp>

namespace QUARK {

class Virtual;
// namespace virtio {
// template <typename, unsigned long> class Console;
// template <typename, unsigned long> class Network;
// } // namespace virtio

// namespace riscv64 {
// class VirtualCPU;
// }

class Payload;
template <> struct Traits<Payload> {
    static constexpr unsigned long Address = Traits<MemoryMap>::RamStart + 128 * 1024;
    static constexpr bool Virtualized   = true;
};

// template <> struct Traits<Virtual> {
//     typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result Serial;
//     typedef Meta::TypeList<virtio::Console<Serial, 0x30000000>, riscv64::VirtualCPU> Devices;
// };

} // namespace QUARK
