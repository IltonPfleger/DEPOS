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

    static void idle() { asm volatile("wfi"); }
    static void halt() { asm volatile("j ."); }
    static auto syscall(auto f) { asm volatile("mv a7, %0\necall" ::"r"(f)); }

    static unsigned long id() {
        unsigned long tp;
        asm volatile("mv %0, tp" : "=r"(tp));
        return tp;
    }

    static void kill() { Atomic::fdec(s_alive); }
    static void barrier() {
        static volatile int ready = 0;
        static volatile bool gsense = true;
        static volatile bool lsense[Traits<::Machine>::CPUS] = {true};

        lsense[CPU::id()] = !lsense[CPU::id()];

        int arrived = Atomic::finc(ready);

        if (arrived == s_alive - 1) {
            Atomic::store(ready, 0);
            Atomic::store(gsense, !Atomic::load(gsense));
        } else {
            while (Atomic::load(gsense) != lsense[CPU::id()])
                asm volatile("nop");
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

    static void init() { KernelMode::init(); }

  private:
    static volatile inline int s_alive = Traits<::Machine>::CPUS;
};
