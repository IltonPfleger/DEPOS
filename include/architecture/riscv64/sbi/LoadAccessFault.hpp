#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/MMU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {

class LoadAccessFault {
    struct NoVirtualDevices {
        using Devices = Meta::TypeList<>;
    };

    using ActiveTraits =
        Meta::IF<Traits<RISCV>::Hypervisor, Traits<Virtual>, NoVirtualDevices>::Result;
    using PageTable = SV39_MMU::PageTable;

    template <typename T> struct Dispatcher;
    template <typename... Ts> struct Dispatcher<Meta::TypeList<Ts...>> {
        static bool run([[maybe_unused]] uintptr_t x, [[maybe_unused]] unsigned int *y) {
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
        uintptr_t addr           = PageTable::virt2phys(csrr<MachineMode::TVAL>());
        unsigned int instruction = *reinterpret_cast<unsigned int *>(PageTable::virt2phys(c->pc));
        unsigned int i           = (instruction >> 7) & 0x1F;
        unsigned int readded     = 0;
        bool response            = Dispatcher<ActiveTraits::Devices>::run(addr, &readded);
        (*c)[i]                  = readded;
        c->pc += 4;
        return response;
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
