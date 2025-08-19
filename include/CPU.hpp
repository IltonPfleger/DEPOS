#pragma once

#include <Machine.hpp>
#include <Spin.hpp>

namespace Timer {
    void handler();
}

namespace Kernel {
    void exception();
}

struct CPU {
    __attribute__((always_inline)) static inline void iret() { asm volatile("mret"); }

    __attribute__((always_inline)) static inline void idle() { asm volatile("wfi"); }

    __attribute__((always_inline)) static inline void stack(void *ptr) { asm volatile("mv sp, %0" ::"r"(ptr)); }

    __attribute__((always_inline)) static inline unsigned int core() {
        unsigned int id;
        asm volatile("csrr %0, mhartid" : "=r"(id));
        return id;
    }

    __attribute__((always_inline)) static inline void *thread() {
        register void *tp asm("tp");
        return tp;
    }

    struct Context {
        uintptr_t ra;
        uintptr_t tp;
        uintptr_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
        uintptr_t a0, a1, a2, a3, a4, a5, a6, a7;
        uintptr_t t0, t1, t2, t3, t4, t5, t6;
        uintptr_t status;
        uintptr_t pc;

        Context(int (*entry)(void *), void *a0, void (*exit)(), void *tp) {
            this->ra     = reinterpret_cast<uintptr_t>(exit);
            this->tp     = reinterpret_cast<uintptr_t>(tp);
            this->pc     = reinterpret_cast<uintptr_t>(entry);
            this->status = reinterpret_cast<uintptr_t>(0ULL | (3 << 11) | (1 << 7));
            this->a0     = reinterpret_cast<uintptr_t>(a0);
        }

        __attribute__((naked)) static void load() {
            asm volatile(
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
                "ld t0, %c[status](sp)\n"
                "ld t1, %c[pc](sp)\n"
                "csrw mstatus, t0\n"
                "csrw mepc, t1\n"
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [tp] "i"(OFFSET_OF(Context, tp)), [s0] "i"(OFFSET_OF(Context, s0)),
                  [s1] "i"(OFFSET_OF(Context, s1)), [a0] "i"(OFFSET_OF(Context, a0)), [s2] "i"(OFFSET_OF(Context, s2)),
                  [s3] "i"(OFFSET_OF(Context, s3)), [s4] "i"(OFFSET_OF(Context, s4)), [s5] "i"(OFFSET_OF(Context, s5)),
                  [s6] "i"(OFFSET_OF(Context, s6)), [s7] "i"(OFFSET_OF(Context, s7)), [s8] "i"(OFFSET_OF(Context, s8)),
                  [s9] "i"(OFFSET_OF(Context, s9)), [s10] "i"(OFFSET_OF(Context, s10)),
                  [s11] "i"(OFFSET_OF(Context, s11)), [status] "i"(OFFSET_OF(Context, status)),
                  [pc] "i"(OFFSET_OF(Context, pc))
                : "memory");
            asm volatile("addi sp, sp, %0" ::"i"(sizeof(Context)));
            CPU::iret();
        }

        __attribute__((naked)) static void swtch(Context **previous, Context *next, Spin *lock) {
            asm volatile("addi sp, sp, %0\n" : : "i"(-sizeof(Context)) : "memory");
            asm volatile(
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
                "sd t0, %c[status](sp)\n"
                "sd ra, %c[pc](sp)"
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [tp] "i"(OFFSET_OF(Context, tp)), [s0] "i"(OFFSET_OF(Context, s0)),
                  [s1] "i"(OFFSET_OF(Context, s1)), [s2] "i"(OFFSET_OF(Context, s2)), [s3] "i"(OFFSET_OF(Context, s3)),
                  [s4] "i"(OFFSET_OF(Context, s4)), [s5] "i"(OFFSET_OF(Context, s5)), [s6] "i"(OFFSET_OF(Context, s6)),
                  [s7] "i"(OFFSET_OF(Context, s7)), [s8] "i"(OFFSET_OF(Context, s8)), [s9] "i"(OFFSET_OF(Context, s9)),
                  [s10] "i"(OFFSET_OF(Context, s10)), [s11] "i"(OFFSET_OF(Context, s11)),
                  [status] "i"(OFFSET_OF(Context, status)), [pc] "i"(OFFSET_OF(Context, pc))
                : "memory");
            asm("sd sp, (%0)" ::"r"(previous));
            asm("mv sp, %0" ::"r"(next));
            lock->release();
            load();
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
                : "i"(-sizeof(Context)), [ra] "i"(OFFSET_OF(Context, ra)), [tp] "i"(OFFSET_OF(Context, tp)),
                  [t0] "i"(OFFSET_OF(Context, t0)), [t1] "i"(OFFSET_OF(Context, t1)), [t2] "i"(OFFSET_OF(Context, t2)),
                  [t3] "i"(OFFSET_OF(Context, t3)), [t4] "i"(OFFSET_OF(Context, t4)), [t5] "i"(OFFSET_OF(Context, t5)),
                  [t6] "i"(OFFSET_OF(Context, t6)), [a0] "i"(OFFSET_OF(Context, a0)), [a1] "i"(OFFSET_OF(Context, a1)),
                  [a2] "i"(OFFSET_OF(Context, a2)), [a3] "i"(OFFSET_OF(Context, a3)), [a4] "i"(OFFSET_OF(Context, a4)),
                  [a5] "i"(OFFSET_OF(Context, a5)), [a6] "i"(OFFSET_OF(Context, a6)), [a7] "i"(OFFSET_OF(Context, a7))
                : "memory");

            asm volatile(
                "csrr t0, mstatus\n"
                "sd t0, %c[status](sp)\n"
                "csrr t0, mepc\n"
                "sd t0, %c[pc](sp)\n" ::[status] "i"(OFFSET_OF(Context, status)),
                [pc] "i"(OFFSET_OF(Context, pc))
                : "memory");
        }

        __attribute__((naked)) static void pop() {
            asm volatile(
                "ld t0, %c[status](sp)\n"
                "csrw mstatus, t0\n"
                "ld t0, %c[pc](sp)\n"
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
                : [ra] "i"(OFFSET_OF(Context, ra)), [tp] "i"(OFFSET_OF(Context, tp)), [t0] "i"(OFFSET_OF(Context, t0)),
                  [t1] "i"(OFFSET_OF(Context, t1)), [t2] "i"(OFFSET_OF(Context, t2)), [t3] "i"(OFFSET_OF(Context, t3)),
                  [t4] "i"(OFFSET_OF(Context, t4)), [t5] "i"(OFFSET_OF(Context, t5)), [t6] "i"(OFFSET_OF(Context, t6)),
                  [a0] "i"(OFFSET_OF(Context, a0)), [a1] "i"(OFFSET_OF(Context, a1)), [a2] "i"(OFFSET_OF(Context, a2)),
                  [a3] "i"(OFFSET_OF(Context, a3)), [a4] "i"(OFFSET_OF(Context, a4)), [a5] "i"(OFFSET_OF(Context, a5)),
                  [a6] "i"(OFFSET_OF(Context, a6)), [a7] "i"(OFFSET_OF(Context, a7)), [pc] "i"(OFFSET_OF(Context, pc)),
                  [status] "i"(OFFSET_OF(Context, status)));

            asm volatile("addi sp, sp, %0\n" : : "i"(sizeof(Context)) : "memory");

            CPU::iret();
        }
    };

    struct Atomic {
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

        static auto mcause() {
            uintmax_t mcause;
            asm("csrr %0, mcause" : "=r"(mcause));
            return mcause;
        }

        static void handler() {
            auto cause        = mcause();
            bool is_interrupt = cause >> (Machine::XLEN - 1);
            auto code         = (cause << 1) >> 1;

            if (is_interrupt) {
                interrupts[code]();
            } else {
                Kernel::exception();
            }
        }

        __attribute__((always_inline)) static inline void set(void (*ptr)()) {
            asm volatile("csrw mtvec, %0" ::"r"(ptr));
        }
    };

    struct Interrupt {
        __attribute__((always_inline)) static inline void disable() { asm("csrci mstatus, 0x8"); }
        __attribute__((always_inline)) static inline void enable() { asm("csrsi mstatus, 0x8"); }

        static void on() { enable(); }
        static bool off() {
            uintmax_t mstatus;
            asm("csrr %0, mstatus" : "=r"(mstatus));
            disable();
            return (mstatus & 0x8) != 0;
        }

        struct Timer {
            static void enable() { asm("li t0, 0x80\ncsrs mie, t0" ::: "t0"); }
            static void disable() { asm("li t0, 0x80\ncsrc mie, t0" ::: "t0"); }
        };
    };
};
