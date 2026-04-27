#ifndef __ARCHITECTURE_COMMON_CPU_HEADER__
#define __ARCHITECTURE_COMMON_CPU_HEADER__

#include <Traits.hpp>
#include <architecture/common/Atomic.hpp>

namespace DEPOS {

namespace ArchitectureCommon {

class CPU {
  public:
    using Atomic = DEPOS::ArchitectureCommon::Atomic;

    static void barrier() {
        static constinit volatile bool gsense = 1;
        static volatile int count             = Traits<DEPOS::CPU>::Active;

        auto sense   = !Atomic::load(gsense);
        int position = Atomic::fdec(count);

        if (position == 1) {
            Atomic::store(count, Traits<DEPOS::CPU>::Active);
            Atomic::store(gsense, sense);
        } else {
            while (Atomic::load(gsense) != sense)
                ;
        }
    }
};

} // namespace ArchitectureCommon

} // namespace DEPOS

#endif
