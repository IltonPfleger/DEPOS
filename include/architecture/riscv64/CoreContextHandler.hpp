#ifndef __RISCV64_CORE_CONTEXT_HANDLER_HEADER__
#define __RISCV64_CORE_CONTEXT_HANDLER_HEADER__

#include <Traits.hpp>
#include <architecture/riscv64/CoreContext.hpp>
#include <architecture/riscv64/Modes.hpp>
#include <architecture/riscv64/csrs.hpp>

namespace DEPOS {

namespace riscv64 {

template <typename T> class CoreContextHandler {

  public:
    static CoreContext *init(size_t id) {
        contexts_[id].core = id;
        return &contexts_[id];
    }

    static void bind(CoreContext *source) { csrw<T::SCRATCH>(source); }
    static CoreContext *current() { return reinterpret_cast<CoreContext *>(csrr<T::SCRATCH>()); }
    static void stack(uintptr_t sp) { current()->ksp = sp; }
    static size_t cpu() { return current()->core; }

  private:
    static inline CoreContext contexts_[Traits<CPU>::Active];
};

} // namespace riscv64

} // namespace DEPOS

#endif
