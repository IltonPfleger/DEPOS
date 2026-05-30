#pragma once

#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/ExceptionHandler.hpp>
#include <architecture/riscv64/MMU.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS {

namespace sbi {

class LoadAccessFault {
    using PageTable = MMU::PageTable;

  public:
    static constexpr unsigned int CODE = 5;

    static void dispatch(size_t id, ContextFrame *c) {
        if (((c->status >> 11) & 0x3) == 1) {
            uintptr_t address        = PageTable::virt2phys(csrr<MachineMode::TVAL>());
            uintptr_t pc             = PageTable::virt2phys(c->pc);
            unsigned int instruction = *reinterpret_cast<unsigned int *>(pc);
            unsigned int i           = (instruction >> 7) & 0x1F;
            if (VirtualCPU::read(address, reinterpret_cast<unsigned int *>(&(*c)[i]))) {
                c->pc += 4;
                return;
            }
        }
        c->value = PageTable::virt2phys(c->value);
        ExceptionHandler::onTrap(id, c);
    }
};

} // namespace sbi

} // namespace DEPOS
