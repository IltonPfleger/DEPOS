#pragma once

#include <architecture/common/Atomic.hpp>
#include <architecture/rv/Context.hpp>
#include <architecture/rv/Interruptions.hpp>
#include <architecture/rv/Modes.hpp>

namespace rv {
class CPU {
  public:
    using Context = ContextBase<rv::KernelMode>;
    using Interruptions = rv::Interruptions;
    using Atomic = ArchitectureCommon::Atomic;

    static auto idle() { asm("wfi"); }
    static auto halt() { asm("1: wfi\n j 1b"); }
    static auto syscall(auto f) { asm("mv a0, %0\necall" ::"r"(f)); }
    static auto id() {
        unsigned long tp;
        asm("mv %0, tp" : "=r"(tp));
        return tp;
    }

    static void barrier(unsigned int cores = Traits<CPUS>::ONLINE) {
        static volatile bool gsense = true;
        static volatile unsigned int ready[2] = {0};

        bool sense = gsense;
        unsigned int arrived = Atomic::finc(ready[sense]);

        if (arrived == cores - 1) {
            ready[sense] = 0;
            gsense = !sense;
        } else {
            while (gsense == sense)
                ;
        }
    }
};
} // namespace rv

namespace rv64 {
using CPU = rv::CPU;
}
