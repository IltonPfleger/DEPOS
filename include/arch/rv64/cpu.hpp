#pragma once

#include <Traits.hpp>
#include <Types.hpp>
#include <arch/rv64/clint.hpp>
#include <arch/rv64/ic.hpp>

class RISCV {
  public:
    struct Machine;
    struct Supervisor;
    struct User;

    using CLINT = SiFiveCLINT;
    using KernelMode = Supervisor;
    using UserMode = Supervisor;

    typedef uintmax_t Register;

#include "Modes.hpp"

    __attribute__((always_inline)) static inline auto syscall(auto f) {
        asm volatile("mv a0, %0" ::"r"(f));
        asm volatile("ecall");
    }

    __attribute__((always_inline)) static inline auto flush() {
        asm volatile("sfence.vma zero, zero");
    }

    template <const int R> static void csrw(auto r) {
        asm volatile("csrw %c0, %1" ::"i"(R), "r"(r));
    }

    template <const int R> static auto csrr() {
        Register r;
        asm volatile("csrr %0, %1" : "=r"(r) : "i"(R));
        return r;
    }

    template <const int R> static void csrs(auto r) {
        asm volatile("csrs %0, %1" ::"i"(R), "r"(r));
    }

    template <const int R> static void csrc(auto r) {
        asm volatile("csrc %0, %1" ::"i"(R), "r"(r));
    }

    static auto core() {
        unsigned int tp;
        asm volatile("mv %0, tp" : "=r"(tp));
        return tp;
    }

    __attribute__((always_inline)) static inline void idle() {
        asm volatile("wfi");
    }

    __attribute__((naked)) static void setup() {
        __asm__ volatile("csrr tp, mhartid\n"
                         "csrr t0, mhartid\n"
                         "mul t0, t0, %1\n"
                         "mv t1, %0\n"
                         "sub sp, t1, t0\n"
                         "ret"
                         :
                         : "r"(Traits<MemoryMap>::RAM_END),
                           "r"(Traits<Memory>::PAGE_SIZE));
    }

    __attribute__((noinline)) static void init() {
        static_assert(!Traits<System>::MULTITASK ||
                      Meta::SAME<KernelMode, Supervisor>::Result);
        static_assert(!Meta::SAME<UserMode, User>::Result ||
                      Traits<System>::MULTITASK);

        if constexpr (Meta::SAME<KernelMode, Supervisor>::Result) {
            if (core() == 0) {
                for (;;)
                    idle();
            }
        }

        if constexpr (Traits<Timer>::Enable) {
            csrs<Machine::IE>(Machine::TI);
            csrs<Supervisor::IE>(Supervisor::TI);
        }

        if constexpr (Meta::SAME<KernelMode, Supervisor>::Result) {
            csrw<Supervisor::TVEC>(SIC::entry);
            csrw<Machine::TVEC>(MIC::entry);
            csrw<Machine::MIDELEG>(0x222);
            csrw<Machine::PMPADDR0>(0x3FFFFFFFFFFFFFULL);
            csrw<Machine::PMPCFG0>(0b11111);
            csrs<Machine::STATUS>(Machine::ME2SUPERVISOR | Machine::PIRQE);
            csrc<Machine::STATUS>(Supervisor::PIRQE | Supervisor::IRQE);
        } else {
            csrs<Machine::STATUS>(Machine::ME2ME);
            csrw<Machine::TVEC>(MIC::entry);
        }

        csrw<Machine::EPC>(__builtin_return_address(0));
        Machine::ret();
    }

#include "Context.hpp"

    class Atomic {
      public:
        template <typename T> static void wait(T &value) {
            while (!__atomic_load_n(&value, __ATOMIC_SEQ_CST))
                ;
        }

        template <typename T> static T clear(T &value) {
            return __atomic_exchange_n(&value, 0, __ATOMIC_SEQ_CST);
        }
    };

    struct Interrupt {
        static void disable() { csrc<KernelMode::STATUS>(KernelMode::IRQE); }
        static void enable() { csrs<KernelMode::STATUS>(KernelMode::IRQE); }

        static void on() { enable(); }
        static bool off() {
            Register status = csrr<KernelMode::STATUS>();
            disable();
            return (status & KernelMode::IRQE) != 0;
        }
    };
};
