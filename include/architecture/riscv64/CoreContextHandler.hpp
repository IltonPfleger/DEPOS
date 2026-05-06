#ifndef __RISCV64_CORE_CONTEXT_HANDLER_HEADER__
#define __RISCV64_CORE_CONTEXT_HANDLER_HEADER__

#include <Traits.hpp>
#include <architecture/riscv64/CPU.hpp>
#include <architecture/riscv64/CoreContext.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/csrs.hpp>

namespace DEPOS {

namespace riscv64 {

template <typename T> class CoreContextHandler {

  public:
    static CoreContext *init(size_t id) {
        s_contexts[id].core = id;
        return &s_contexts[id];
    }

    static void bind(CoreContext *source) { csrw<T::SCRATCH>(source); }
    static void stack(uintptr_t sp) { reinterpret_cast<CoreContext *>(csrr<T::SCRATCH>())->ksp = sp; }
    static size_t cpu() { return reinterpret_cast<CoreContext *>(csrr<T::SCRATCH>())->core; }

  private:
    static inline CoreContext s_contexts[Traits<CPU>::Active];
};

} // namespace riscv64

} // namespace DEPOS

#endif
