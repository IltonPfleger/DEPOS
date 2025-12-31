#pragma once

class CPU {
  public:
    using Context = RV64::Context;
    using Interruptions = RV64::Interruptions;
    using Atomic = ArchitectureCommon::Atomic;
    class TLB {
      public:
        static auto flush() { asm volatile("sfence.vma zero, zero"); }
    };

    static auto idle() { asm volatile("wfi"); }
    static auto halt() { asm volatile("j ."); }
    static auto syscall(auto f) { asm volatile("mv a0, %0\necall" ::"r"(f)); }
    static auto id() {
        unsigned long tp;
        asm volatile("mv %0, tp" : "=r"(tp));
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

    /* *** Boot Functions *** */
    __attribute__((naked)) static void probe() {
        unsigned long core;
        asm volatile("csrr tp, mhartid\nmv %0, tp" : "=r"(core));
        uintptr_t addr = Traits<MemoryMap>::RAM_END - Traits<Memory>::PAGE_SIZE * core;
        asm("mv sp, %0" ::"r"(addr));
        asm volatile("ret");
    }

    __attribute__((noinline)) static void jmode() {
        static_assert(!Traits<System>::MULTITASK || Meta::SAME<KernelMode, SupervisorMode>::Result);

        if constexpr (Traits<System>::MULTITASK) {
            if (!(csrr<MachineMode::MISA>() & (1UL << ('S' - 'A')))) {
                for (;;)
                    CPU::idle();
            }
        }

        if constexpr (Traits<Timer>::Enable) {
            csrs<MachineMode::IE>(MachineMode::TI);
        }

        if constexpr (Meta::SAME<KernelMode, SupervisorMode>::Result) {
            csrw<SupervisorMode::TVEC>(SIC::entry);
            csrw<MachineMode::TVEC>(MIC::entry);
            csrw<MachineMode::MIDELEG>(0x222);
            csrw<MachineMode::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            csrw<MachineMode::PMPCFG0>(0b11111);
            csrs<MachineMode::STATUS>(MachineMode::ME2SUPERVISOR | MachineMode::PIRQE);
            csrc<MachineMode::STATUS>(SupervisorMode::PIRQE | SupervisorMode::IRQE);
        } else {
            csrs<MachineMode::STATUS>(MachineMode::ME2ME);
            csrw<MachineMode::TVEC>(MIC::entry);
        }

        csrw<MachineMode::EPC>(__builtin_return_address(0));
        MachineMode::ret();
    }

    static void init() {
        if constexpr (Traits<Timer>::Enable && Traits<System>::MULTITASK) {
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
        }
    }
};
