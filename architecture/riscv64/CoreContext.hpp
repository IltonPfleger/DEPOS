#ifndef __DEPOS_RISCV64_CORE_CONTEXT__
#define __DEPOS_RISCV64_CORE_CONTEXT__

#include <types.hpp>

namespace DEPOS {

struct CoreContext {
    uintptr_t scratch0;
    uintptr_t scratch1;
    uintptr_t ksp;
    uintptr_t core;
};

} // namespace DEPOS

#endif
