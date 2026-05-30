#ifndef __DEPOS_RISCV64_IPI__
#define __DEPOS_RISCV64_IPI__

#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS {

class IPI {
  public:
    static void onTrap(size_t, ContextFrame *) {
        VirtualCPU::onInterProcessorInterrupt();
        CLINT::ipi(mhartid(), true);
    }
};

} // namespace DEPOS

#endif
