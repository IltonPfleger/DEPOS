#ifndef __DEPOS_RISCV64_PMU__
#define __DEPOS_RISCV64_PMU__

#include <architecture/riscv64/csrs.hpp>

namespace DEPOS::riscv64 {

class PMU {

  public:
    typedef size_t Event;
    enum : Event {
        L_FIRST = 0,
        L_INSTRUCTIONS,
        L_LAST,
        G_FIRST,
        G_LAST,
    };

    static uintmax_t read(Event event) {
        switch (event) {
            case L_INSTRUCTIONS: return csrr<INSTRET>();
            default: return UNKNOWN;
        }
    }

  private:
    static constexpr int INSTRET       = 0xC02;
    static constexpr uintmax_t UNKNOWN = ~0ULL;
};

} // namespace DEPOS::riscv64

#endif
