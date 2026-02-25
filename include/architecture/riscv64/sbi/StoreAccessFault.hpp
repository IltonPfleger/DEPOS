#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <drivers/virtio/Console.hpp>
#include <drivers/virtio/Network.hpp>

namespace riscv64 {

namespace sbi {

class StoreAccessFault {

    struct NoVirtualDevices {
        static Meta::TypeList<> Devices;
    };

    using ActiveTraits = Meta::IF<Traits<RISCV>::Hypervisor, Traits<Virtual>, NoVirtualDevices>::Result;
    using PageTable = SV39_MMU::PageTable;

  public:
    static constexpr unsigned int CODE = 7;

    template <typename T> struct Dispatcher;
    template <typename... Ts> struct Dispatcher<Meta::TypeList<Ts...>> {
        static bool run(uintptr_t x, unsigned int y) {
            return ([&]() {
                if (x >= Ts::Address && x < (Ts::Address + Ts::Size)) {
                    return Ts::write(x, y);
                }
                return false;
            }() || ...);
        }
    };

    static bool handler(MachineContext *c) {
        uintptr_t addr = PageTable::virt2phys(csrr<MachineMode::TVAL>());
        unsigned int instruction = *reinterpret_cast<unsigned int *>(PageTable::virt2phys(c->pc));
        unsigned int i = (instruction >> 20) & 0x1F;
        return c->pc += 4, Dispatcher<ActiveTraits::Devices>::run(addr, (*c)[i]);
    }
};

} // namespace sbi

} // namespace riscv64
