#pragma once

#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/ExceptionHandler.hpp>
#include <architecture/riscv64/Modes.hpp>

namespace QUARK::sbi {

class StoreAccessFault {
    using PageTable = MMU::PageTable;

  public:
    static constexpr unsigned int CODE = 7;

    static void dispatch(ContextFrame *c) {
        if (((c->status >> 11) & 0x3) == 1) {
            uintptr_t address        = PageTable::virt2phys(csrr<MachineMode::TVAL>());
            unsigned int instruction = *reinterpret_cast<unsigned int *>(PageTable::virt2phys(c->pc));
            unsigned int i           = (instruction >> 20) & 0x1F;
            uintmax_t source         = (*c)[i];
            if (VirtualCPU::write(address, source)) {
                c->pc += 4;
                return;
            }
        }
        ExceptionHandler::onTrap(c);
    }
};

} // namespace QUARK::sbi
