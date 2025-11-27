#pragma once
class CPU {
  public:
    using Context = RV64::Context;
    using Interruptions = RV64::Interruptions;
    using Atomic = RV64::Atomic;
    class TLB {
      public:
        static auto flush() { asm volatile("sfence.vma zero, zero"); }
    };

    static auto idle() { asm volatile("wfi"); }
    static auto halt() { asm volatile("j ."); }
    static auto syscall(auto f) { asm volatile("mv a0, %0\necall" ::"r"(f)); }

    static unsigned long id() {
        unsigned long tp;
        asm volatile("mv %0, tp" : "=r"(tp));
        return tp;
    }

    static void kill() { Atomic::fdec(s_alive); }
    static void barrier() {
        static volatile int ready[2] = {0, 0};
        static volatile int i;

        int j = i;

        Atomic::finc(ready[j]);

        if (CPU::id() == Traits<::Machine>::BSP) {
            while (ready[j] < s_alive)
                ;
            i = !i;
            ready[j] = 0;
        } else {
            while (ready[j])
                ;
        }
    }

    /* *** Boot Functions *** */
    __attribute__((naked)) static void setup() {
        unsigned long core;
        asm volatile("csrr tp, mhartid\nmv %0, tp" : "=r"(core));
        uintptr_t addr = Traits<MemoryMap>::PhysicalRamEnd - Traits<Memory>::PAGE_SIZE * core;
        asm("mv sp, %0" ::"r"(addr));
        asm volatile("ret");
    }

    __attribute__((noinline)) static void init() {
        static_assert(!Traits<System>::MULTITASK || Meta::SAME<KernelMode, Supervisor>::Result);

        csrc<Machine::STATUS>(Machine::IRQE);

        if constexpr (Meta::SAME<KernelMode, Supervisor>::Result) {
            if (!(csrr<Machine::MISA>() & (1UL << ('S' - 'A')))) {
                kill();
                for (;;)
                    CPU::idle();
            }
        }

        if constexpr (Traits<Timer>::Enable) {
            csrs<Machine::IE>(Machine::TI);
        }

        if constexpr (Meta::SAME<KernelMode, Supervisor>::Result) {
            csrw<Supervisor::TVEC>(Supervisor::IC::entry);
            csrw<Machine::TVEC>(Supervisor::Firmware::entry);
            csrw<Machine::MIDELEG>(0x222);
            csrw<Machine::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            csrw<Machine::PMPCFG0>(0b11111);
            csrs<Machine::STATUS>(Machine::ME2SUPERVISOR | Machine::PIRQE);
            csrc<Machine::STATUS>(Supervisor::PIRQE);
        } else {
            csrs<Machine::STATUS>(Machine::ME2ME);
            csrw<Machine::TVEC>(Machine::IC::entry);
        }

        csrw<Machine::EPC>(__builtin_return_address(0));
        Machine::ret();
    }

  private:
    static volatile inline int s_alive = Traits<::Machine>::CPUS;
};
