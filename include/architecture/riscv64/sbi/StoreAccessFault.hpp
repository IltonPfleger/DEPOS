#pragma once

#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/ExceptionHandler.hpp>
#include <architecture/riscv64/Modes.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {

class StoreAccessFault {
    using PageTable = MMU::PageTable;

  public:
    static constexpr unsigned int CODE = 7;

    static void dispatch(size_t id, ContextFrame *c) {
        if (((c->status >> 11) & 0x3) == 1) {
            uintptr_t address        = PageTable::virt2phys(csrr<MachineMode::TVAL>());
            unsigned int instruction = *reinterpret_cast<unsigned int *>(PageTable::virt2phys(c->pc));
            unsigned int i           = (instruction >> 20) & 0x1F;
            if (VirtualCPU::write(address, (*c)[i])) {
                c->pc += 4;
                return;
            }
        }
        ExceptionHandler::onTrap(id, c);
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
