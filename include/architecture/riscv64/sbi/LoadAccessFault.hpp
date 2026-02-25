#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/MMU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace riscv64 {

namespace sbi {

class LoadAccessFault {
    struct NoVirtualDevices {
        static Meta::TypeList<> Devices;
    };

    using ActiveTraits = Meta::IF<Traits<RISCV>::Hypervisor, Traits<Virtual>, NoVirtualDevices>::Result;
    using PageTable = SV39_MMU::PageTable;

    template <typename T> struct Dispatcher;
    template <typename... Ts> struct Dispatcher<Meta::TypeList<Ts...>> {
        static bool run(uintptr_t x, unsigned int *y) {
            return ([&]() {
                if (x >= Ts::Address && x < (Ts::Address + Ts::Size)) {
                    return Ts::read(x, y);
                }
                return false;
            }() || ...);
        }
    };

  public:
    static constexpr unsigned int CODE = 5;

    static bool handler(MachineContext *c) {
        uintptr_t addr = PageTable::virt2phys(csrr<MachineMode::TVAL>());
        unsigned int instruction = *reinterpret_cast<unsigned int *>(PageTable::virt2phys(c->pc));
        unsigned int i = (instruction >> 7) & 0x1F;
        return c->pc += 4, Dispatcher<ActiveTraits::Devices>::run(addr, reinterpret_cast<unsigned int*>(&(*c)[i]));
    }
};

} // namespace sbi

} // namespace riscv64
