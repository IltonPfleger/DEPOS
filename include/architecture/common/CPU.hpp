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
        __attribute__((section(".data"))) static volatile unsigned char gsense = 0;
        __attribute__((section(".data"))) static volatile uint32_t ready[2]    = {0};

        unsigned char sense = gsense;
        uint32_t arrived    = Atomic::finc(ready[sense]);

        if (arrived == Traits<DEPOS::CPU>::Active - 1) {
            ready[sense] = 0;
            gsense       = !sense;
        } else {
            while (gsense == sense)
                ;
        }
    }
};

} // namespace ArchitectureCommon

} // namespace DEPOS

#endif
