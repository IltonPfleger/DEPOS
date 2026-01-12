#pragma once

class CPU {
  public:
    using Context = ContextBase<KernelMode>;
    using Interruptions = RV64::Interruptions;
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

    /* *** Boot Functions *** */
    __attribute__((naked)) static void probe() {
        unsigned long core;
        asm("csrr tp, mhartid\nmv %0, tp" : "=r"(core));
        asm("mv sp, %0" ::"r"(Traits<MemoryMap>::RamEnd - Traits<Memory>::PageSize * core));
        asm("ret");
    }

    __attribute__((naked)) static void jmode() {
        if constexpr (Traits<System>::MULTITASK) {
            if (!(csrr<MachineMode::MISA>() & (1UL << ('S' - 'A')))) {
                for (;;)
                    CPU::idle();
            }
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
            csrc<MachineMode::STATUS>(MachineMode::PIRQE);
            csrw<MachineMode::TVEC>(MIC::entry);
        }

        if constexpr (Traits<Timer>::Enable) {
            csrs<MachineMode::IE>(MachineMode::TI);
        }

        csrw<MachineMode::EPC>(__builtin_return_address(0));
        MachineMode::ret();
        __builtin_unreachable();
    }

    __attribute__((noinline)) static void init() {
        jmode();
        if constexpr (Traits<Timer>::Enable && Traits<System>::MULTITASK) {
            csrs<SupervisorMode::IE>(SupervisorMode::TI);
        }
    }
};
