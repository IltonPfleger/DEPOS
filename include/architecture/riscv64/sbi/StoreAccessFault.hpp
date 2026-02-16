#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <drivers/virtio/Console.hpp>

namespace riscv64 {

namespace sbi {

class StoreAccessFault {
    using ActiveTraits = Meta::IF<Traits<RISCV>::Hypervisor, Traits<Virtual>, Traits<Dummy>>::Result;
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
        if (dispatch(addr, (*c)[i], ActiveTraits::Devices{})) return c->pc += 4, true;
        return false;
        // if (virtio::Handler::write(addr, (*c)[rs2])) {
        //     c->pc += 4;
        //     return true;
        // }
        // return false;
    }
};

} // namespace sbi

} // namespace riscv64
