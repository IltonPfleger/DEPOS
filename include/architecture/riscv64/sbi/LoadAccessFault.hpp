#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/MMU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>
// #include <drivers/virtio/Handler.hpp>

namespace riscv64 {

namespace sbi {

class LoadAccessFault {
    using ActiveTraits = Meta::IF<Traits<RISCV>::Hypervisor, Traits<Virtual>, Traits<Dummy>>::Result;
    using PageTable = SV39_MMU::PageTable;

    template <typename... Ts>
    static bool dispatch([[maybe_unused]] uintptr_t x, [[maybe_unused]] unsigned int *y, Meta::TypeList<Ts...>) {
        return ([&]() {
            if (x >= Ts::Address && x < (Ts::Address + Ts::Size)) {
                return Ts::read(x, y);
            }
            return false;
        }() || ...);
    }

  public:
    static constexpr unsigned int CODE = 5;

    static bool handler(MachineContext *c) {
        uintptr_t addr = PageTable::virt2phys(csrr<MachineMode::TVAL>());
        unsigned int instruction = *reinterpret_cast<unsigned int *>(PageTable::virt2phys(c->pc));
        unsigned int i = (instruction >> 7) & 0x1F;
        if (dispatch(addr, reinterpret_cast<unsigned int *>(&(*c)[i]), ActiveTraits::Devices{})) return c->pc += 4, true;
        return false;
    }
};

} // namespace sbi

} // namespace riscv64
