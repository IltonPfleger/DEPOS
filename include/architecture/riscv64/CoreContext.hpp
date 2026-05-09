#ifndef __DEPOS_RISCV64_CORE_CONTEXT__
#define __DEPOS_RISCV64_CORE_CONTEXT__

#include <types.hpp>

namespace DEPOS {

namespace riscv64 {

struct CoreContext {
    uintptr_t scratch0;
    uintptr_t scratch1;
    uintptr_t ksp;
    uintptr_t core;
};

} // namespace riscv64

} // namespace DEPOS

#endif
