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

    template <typename... Ts>
    static bool dispatch([[maybe_unused]] uintptr_t x, [[maybe_unused]] unsigned int y, Meta::TypeList<Ts...>) {
        return ([&]() {
            if (x >= Ts::Address && x < (Ts::Address + Ts::Size)) {
                return Ts::write(x, y);
            }
            return false;
        }() || ...);
    }

    static bool handler(MachineContext *c) {
        uintptr_t addr = PageTable::virt2phys(csrr<MachineMode::TVAL>());
        unsigned int instruction = *reinterpret_cast<unsigned int *>(PageTable::virt2phys(c->pc));
        unsigned int i = (instruction >> 20) & 0x1F;
        return c->pc += 4, dispatch(addr, (*c)[i], ActiveTraits::Devices);
    }
};

} // namespace sbi

} // namespace riscv64
