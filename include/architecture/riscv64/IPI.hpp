#ifndef __DEPOS_RISCV64_IPI__
#define __DEPOS_RISCV64_IPI__

#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/VCPU.hpp>

namespace DEPOS::riscv64 {

class IPI {
  public:
    static void onTrap(size_t, Context *) {
        VCPU::onInterProcessorInterrupt();
        CLINT::ipi(mhartid(), true);
    }
};

} // namespace DEPOS::riscv64

#endif
