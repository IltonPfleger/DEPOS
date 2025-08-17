#pragma once

#include <Machine.hpp>
#define OFFSET(type, member) ((unsigned long)&(((type *)0)->member))

namespace Timer {
    void handler();
}

namespace Kernel {
    void exception();
}

struct CPU {
    __attribute__((always_inline)) static inline void iret() { __asm__ volatile("mret"); }

    __attribute__((always_inline)) static inline void idle() { __asm__ volatile("wfi"); }

    __attribute__((always_inline)) static inline void stack(void *ptr) { __asm__ volatile("mv sp, %0" ::"r"(ptr)); }

    __attribute__((naked)) static unsigned int core() { __asm__ volatile("csrr a0, mhartid\nret"); }

    __attribute__((naked)) static void *thread() { __asm__ volatile("mv a0, tp\nret"); }

    struct Context {
        struct Saved {
            uintptr_t ra;
            uintptr_t tp;
            uintptr_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
            uintptr_t epc;
            uintptr_t estatus;
        } saved;
        uintptr_t t0, t1, t2, t3, t4, t5, t6;
        uintptr_t a0, a1, a2, a3, a4, a5, a6, a7;

        Context(int (*entry)(void *), void *a0, void (*exit)(), void *tp) {
            this->saved.ra      = reinterpret_cast<uintptr_t>(exit);
            this->saved.tp      = reinterpret_cast<uintptr_t>(tp);
            this->saved.epc     = reinterpret_cast<uintptr_t>(entry);
            this->saved.estatus = reinterpret_cast<uintptr_t>(0ULL | (3 << 11) | (1 << 7));
            this->a0            = reinterpret_cast<uintptr_t>(a0);
        }

        __attribute__((always_inline)) static inline void save() {
            asm volatile(
                "addi sp, sp, %0\n"
                "sd ra, %c[ra](sp)\n"
                "sd tp, %c[tp](sp)\n"
                "sd s0, %c[s0](sp)\n"
                "sd s1, %c[s1](sp)\n"
                "sd s2, %c[s2](sp)\n"
                "sd s3, %c[s3](sp)\n"
                "sd s4, %c[s4](sp)\n"
                "sd s5, %c[s5](sp)\n"
                "sd s6, %c[s6](sp)\n"
                "sd s7, %c[s7](sp)\n"
                "sd s8, %c[s8](sp)\n"
                "sd s9, %c[s9](sp)\n"
                "sd s10, %c[s10](sp)\n"
                "sd s11, %c[s11](sp)\n"
                "csrr t0, mstatus\n"
                "andi t0, t0, ~0x80\n"
                "li t1, 0x1800\n"
                "or t0, t0, t1\n"
                "sd t0, %c[estatus](sp)\n"
                "sd ra, %c[epc](sp)"
                :
                : "i"(-sizeof(Context::Saved)), [ra] "i"(OFFSET(Context, saved.ra)),
                  [tp] "i"(OFFSET(Context, saved.tp)), [s0] "i"(OFFSET(Context, saved.s0)),
                  [s1] "i"(OFFSET(Context, saved.s1)), [s2] "i"(OFFSET(Context, saved.s2)),
                  [s3] "i"(OFFSET(Context, saved.s3)), [s4] "i"(OFFSET(Context, saved.s4)),
                  [s5] "i"(OFFSET(Context, saved.s5)), [s6] "i"(OFFSET(Context, saved.s6)),
                  [s7] "i"(OFFSET(Context, saved.s7)), [s8] "i"(OFFSET(Context, saved.s8)),
                  [s9] "i"(OFFSET(Context, saved.s9)), [s10] "i"(OFFSET(Context, saved.s10)),
                  [s11] "i"(OFFSET(Context, saved.s11)), [estatus] "i"(OFFSET(Context, saved.estatus)),
                  [epc] "i"(OFFSET(Context, saved.epc))
                : "cc", "memory");
        }

        __attribute__((naked)) static void load(Context *c) {
            asm volatile(
                "mv sp, %0\n"
                "ld ra, %c[ra](sp)\n"
                "ld tp, %c[tp](sp)\n"
                "ld s0, %c[s0](sp)\n"
                "ld a0, %c[a0](sp)\n"
                "ld s1, %c[s1](sp)\n"
                "ld s2, %c[s2](sp)\n"
                "ld s3, %c[s3](sp)\n"
                "ld s4, %c[s4](sp)\n"
                "ld s5, %c[s5](sp)\n"
                "ld s6, %c[s6](sp)\n"
                "ld s7, %c[s7](sp)\n"
                "ld s8, %c[s8](sp)\n"
                "ld s9, %c[s9](sp)\n"
                "ld s10, %c[s10](sp)\n"
                "ld s11, %c[s11](sp)\n"
                "ld t0, %c[estatus](sp)\n"
                "ld t1, %c[epc](sp)\n"
                "csrw mstatus, t0\n"
                "csrw mepc, t1\n"
                "addi sp, sp, %1\n"
                :
                : "r"(c), "i"(sizeof(Context::Saved)), [ra] "i"(OFFSET(Context, saved.ra)),
                  [tp] "i"(OFFSET(Context, saved.tp)), [s0] "i"(OFFSET(Context, saved.s0)),
                  [s1] "i"(OFFSET(Context, saved.s1)), [a0] "i"(OFFSET(Context, a0)),
                  [s2] "i"(OFFSET(Context, saved.s2)), [s3] "i"(OFFSET(Context, saved.s3)),
                  [s4] "i"(OFFSET(Context, saved.s4)), [s5] "i"(OFFSET(Context, saved.s5)),
                  [s6] "i"(OFFSET(Context, saved.s6)), [s7] "i"(OFFSET(Context, saved.s7)),
                  [s8] "i"(OFFSET(Context, saved.s8)), [s9] "i"(OFFSET(Context, saved.s9)),
                  [s10] "i"(OFFSET(Context, saved.s10)), [s11] "i"(OFFSET(Context, saved.s11)),
                  [estatus] "i"(OFFSET(Context, saved.estatus)), [epc] "i"(OFFSET(Context, saved.epc))
                : "memory", "cc");
            CPU::iret();
        }

        __attribute__((always_inline)) static inline void push() {
            asm volatile(
                "addi sp, sp, %0\n"
                "sd ra, %c[ra](sp)\n"
                "sd tp, %c[tp](sp)\n"
                "sd t0, %c[t0](sp)\n"
                "sd t1, %c[t1](sp)\n"
                "sd t2, %c[t2](sp)\n"
                "sd t3, %c[t3](sp)\n"
                "sd t4, %c[t4](sp)\n"
                "sd t5, %c[t5](sp)\n"
                "sd t6, %c[t6](sp)\n"
                "sd a0, %c[a0](sp)\n"
                "sd a1, %c[a1](sp)\n"
                "sd a2, %c[a2](sp)\n"
                "sd a3, %c[a3](sp)\n"
                "sd a4, %c[a4](sp)\n"
                "sd a5, %c[a5](sp)\n"
                "sd a6, %c[a6](sp)\n"
                "sd a7, %c[a7](sp)\n"
                :
                : "i"(-sizeof(Context)), [ra] "i"(OFFSET(Context, saved.ra)), [tp] "i"(OFFSET(Context, saved.tp)),
                  [t0] "i"(OFFSET(Context, t0)), [t1] "i"(OFFSET(Context, t1)), [t2] "i"(OFFSET(Context, t2)),
                  [t3] "i"(OFFSET(Context, t3)), [t4] "i"(OFFSET(Context, t4)), [t5] "i"(OFFSET(Context, t5)),
                  [t6] "i"(OFFSET(Context, t6)), [a0] "i"(OFFSET(Context, a0)), [a1] "i"(OFFSET(Context, a1)),
                  [a2] "i"(OFFSET(Context, a2)), [a3] "i"(OFFSET(Context, a3)), [a4] "i"(OFFSET(Context, a4)),
                  [a5] "i"(OFFSET(Context, a5)), [a6] "i"(OFFSET(Context, a6)), [a7] "i"(OFFSET(Context, a7))
                : "memory");

            asm volatile(
                "sd s0, %c[s0](sp)\n"
                "sd s1, %c[s1](sp)\n"
                "sd s2, %c[s2](sp)\n"
                "sd s3, %c[s3](sp)\n"
                "sd s4, %c[s4](sp)\n"
                "sd s5, %c[s5](sp)\n"
                "sd s6, %c[s6](sp)\n"
                "sd s7, %c[s7](sp)\n"
                "sd s8, %c[s8](sp)\n"
                "sd s9, %c[s9](sp)\n"
                "sd s10, %c[s10](sp)\n"
                "sd s11, %c[s11](sp)\n"
                "csrr t0, mstatus\n"
                "sd t0, %c[estatus](sp)\n"
                "csrr t0, mepc\n"
                "sd t0, %c[epc](sp)\n" ::[s0] "i"(OFFSET(Context, saved.s0)),
                [s1] "i"(OFFSET(Context, saved.s1)), [s2] "i"(OFFSET(Context, saved.s2)),
                [s3] "i"(OFFSET(Context, saved.s3)), [s4] "i"(OFFSET(Context, saved.s4)),
                [s5] "i"(OFFSET(Context, saved.s5)), [s6] "i"(OFFSET(Context, saved.s6)),
                [s7] "i"(OFFSET(Context, saved.s7)), [s8] "i"(OFFSET(Context, saved.s8)),
                [s9] "i"(OFFSET(Context, saved.s9)), [s10] "i"(OFFSET(Context, saved.s10)),
                [s11] "i"(OFFSET(Context, saved.s11)), [estatus] "i"(OFFSET(Context, saved.estatus)),
                [epc] "i"(OFFSET(Context, saved.epc))
                : "memory");
        }

        __attribute__((naked)) static void pop() {
            asm volatile(
                "ld t0, %c[estatus](sp)\n"
                "csrw mstatus, t0\n"
                "ld t0, %c[epc](sp)\n"
                "csrw mepc, t0\n"
                "ld ra, %c[ra](sp)\n"
                "ld tp, %c[tp](sp)\n"
                "ld t0, %c[t0](sp)\n"
                "ld t1, %c[t1](sp)\n"
                "ld t2, %c[t2](sp)\n"
                "ld t3, %c[t3](sp)\n"
                "ld t4, %c[t4](sp)\n"
                "ld t5, %c[t5](sp)\n"
                "ld t6, %c[t6](sp)\n"
                "ld a0, %c[a0](sp)\n"
                "ld a1, %c[a1](sp)\n"
                "ld a2, %c[a2](sp)\n"
                "ld a3, %c[a3](sp)\n"
                "ld a4, %c[a4](sp)\n"
                "ld a5, %c[a5](sp)\n"
                "ld a6, %c[a6](sp)\n"
                "ld a7, %c[a7](sp)\n"
                :
                : [ra] "i"(OFFSET(Context, saved.ra)), [tp] "i"(OFFSET(Context, saved.tp)),
                  [t0] "i"(OFFSET(Context, t0)), [t1] "i"(OFFSET(Context, t1)), [t2] "i"(OFFSET(Context, t2)),
                  [t3] "i"(OFFSET(Context, t3)), [t4] "i"(OFFSET(Context, t4)), [t5] "i"(OFFSET(Context, t5)),
                  [t6] "i"(OFFSET(Context, t6)), [a0] "i"(OFFSET(Context, a0)), [a1] "i"(OFFSET(Context, a1)),
                  [a2] "i"(OFFSET(Context, a2)), [a3] "i"(OFFSET(Context, a3)), [a4] "i"(OFFSET(Context, a4)),
                  [a5] "i"(OFFSET(Context, a5)), [a6] "i"(OFFSET(Context, a6)), [a7] "i"(OFFSET(Context, a7)),
                  [epc] "i"(OFFSET(Context, saved.epc)), [estatus] "i"(OFFSET(Context, saved.estatus)));

            asm volatile(
                "ld s0, %c[s0](sp)\n"
                "ld s1, %c[s1](sp)\n"
                "ld s2, %c[s2](sp)\n"
                "ld s3, %c[s3](sp)\n"
                "ld s4, %c[s4](sp)\n"
                "ld s5, %c[s5](sp)\n"
                "ld s6, %c[s6](sp)\n"
                "ld s7, %c[s7](sp)\n"
                "ld s8, %c[s8](sp)\n"
                "ld s9, %c[s9](sp)\n"
                "ld s10, %c[s10](sp)\n"
                "ld s11, %c[s11](sp)\n"
                "addi sp, sp, %0\n"
                :
                : "i"(sizeof(Context)), [s0] "i"(OFFSET(Context, saved.s0)), [s1] "i"(OFFSET(Context, saved.s1)),
                  [s2] "i"(OFFSET(Context, saved.s2)), [s3] "i"(OFFSET(Context, saved.s3)),
                  [s4] "i"(OFFSET(Context, saved.s4)), [s5] "i"(OFFSET(Context, saved.s5)),
                  [s6] "i"(OFFSET(Context, saved.s6)), [s7] "i"(OFFSET(Context, saved.s7)),
                  [s8] "i"(OFFSET(Context, saved.s8)), [s9] "i"(OFFSET(Context, saved.s9)),
                  [s10] "i"(OFFSET(Context, saved.s10)), [s11] "i"(OFFSET(Context, saved.s11))
                : "memory");

            CPU::iret();
        }

        __attribute__((always_inline)) [[nodiscard]] static inline Context *get() {
            Context *sp;
            __asm__ volatile("mv %0, sp" : "=r"(sp));
            return sp;
        }
    };

    struct Atomic {
        static int tsl(volatile int &value) { return __atomic_test_and_set(&value, __ATOMIC_SEQ_CST); }
        static int fdec(volatile int &value) { return __atomic_fetch_sub(&value, 1, __ATOMIC_SEQ_CST); }
        static int finc(volatile int &value) { return __atomic_fetch_add(&value, 1, __ATOMIC_SEQ_CST); }
    };

    struct Trap {
        using Handler = void (*)();

        static inline Handler interrupts[8] = {
            nullptr,         // 0
            nullptr,         // 1
            nullptr,         // 2
            nullptr,         // 3
            nullptr,         // 4
            nullptr,         // 5
            nullptr,         // 6
            Timer::handler,  // 7
        };

        __attribute__((naked)) static uintmax_t icause() { __asm__ volatile("csrr a0, mcause\nret"); }

        static void handler() {
            auto cause        = icause();
            bool is_interrupt = cause >> (Machine::XLEN - 1);
            auto code         = (cause << 1) >> 1;

            if (is_interrupt) {
                interrupts[code]();
            } else {
                Kernel::exception();
            }
        }

        __attribute__((always_inline)) static inline void set(void (*ptr)()) {
            __asm__ volatile("csrw mtvec, %0" ::"r"(ptr));
        }
    };

    struct Interrupt {
        __attribute__((always_inline)) static inline void disable() { __asm__ volatile("csrci mstatus, 0x8"); }
        __attribute__((always_inline)) static inline void enable() { __asm__ volatile("csrsi mstatus, 0x8"); }

        struct Timer {
            static void enable() { __asm__ volatile("li t0, 0x80\ncsrs mie, t0" ::: "t0"); }
            static void disable() { __asm__ volatile("li t0, 0x80\ncsrc mie, t0" ::: "t0"); }
        };
    };
};
