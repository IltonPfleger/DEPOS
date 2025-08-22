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
    typedef uintmax_t Register;
    enum class Mode { SUPERVISOR, MACHINE };

    static constexpr Mode MODE = Mode::MACHINE;

    static constexpr const int MIE = 0x304;
    static constexpr const int SIE = 0x104;
    static constexpr const int KIE = MODE == Mode::SUPERVISOR ? SIE : MIE;

    static constexpr const int MSTATUS = 0x300;
    static constexpr const int SSTATUS = 0x100;
    static constexpr const int KSTATUS = MODE == Mode::SUPERVISOR ? SSTATUS : MSTATUS;

    static constexpr const int MEPC = 0x341;
    static constexpr const int SEPC = 0x141;
    static constexpr const int KEPC = MODE == Mode::SUPERVISOR ? SEPC : MEPC;

    static constexpr const int MCAUSE = 0x342;
    static constexpr const int SCAUSE = 0x142;
    static constexpr const int KCAUSE = MODE == Mode::SUPERVISOR ? SCAUSE : MCAUSE;

    static constexpr const int MTVEC = 0x305;
    static constexpr const int STVEC = 0x105;

    enum : Register {
        SUPERVISOR2SUPERVISOR = 1 << 8,
        MACHINE2SUPERVISOR    = 1 << 11,
        MACHINE2MACHINE       = 3 << 11,
        KERNEL2KERNEL         = MODE == Mode::SUPERVISOR ? SUPERVISOR2SUPERVISOR : MACHINE2MACHINE,
        KERNEL2USER           = MODE == Mode::SUPERVISOR ? SUPERVISOR2SUPERVISOR : MACHINE2MACHINE,
        MACHINE_IRQ           = 1 << 3,
        SUPERVISOR_IRQ        = 1 << 1,
        KERNEL_IRQ            = MODE == Mode::SUPERVISOR ? SUPERVISOR_IRQ : MACHINE_IRQ,

        MPP  = 3 << 11,                                 // Machine Previous Previlege Mask
        UPIE = 1 << 4,                                  // User Previous Interrupt Enable
        SPIE = 1 << 5,                                  // Supervisor Previous Interrupt Enable
        MPIE = 1 << 7,                                  // Machine Previous Interrupt Enable
        KPIE = MODE == Mode::SUPERVISOR ? SPIE : MPIE,  // Kernel Previous Interrupt Enable
        UTIE = 1ULL << 4,                               // User Timer Interrupt Enable
        STIE = 1ULL << 5,                               // Supervisor Timer Interrupt Enable
        MTIE = 1ULL << 7,                               // Machine Timer Interrupt Enable
        KTIE = MODE == Mode::SUPERVISOR ? STIE : MTIE,  // Kernel Timer Interrupt Enable
    };

    template <const int R>
    __attribute__((always_inline)) static inline void csrw(Register r) {
        asm volatile("csrw %0, %1" ::"i"(R), "r"(r));
    }

    template <const int R>
    __attribute__((always_inline)) static inline auto csrr() {
        Register r;
        asm volatile("csrr %0, %1" : "=r"(r) : "i"(R));
        return r;
    }

    template <const int R>
    __attribute__((always_inline)) static inline void csrs(Register r) {
        asm volatile("csrs %0, %1" ::"i"(R), "r"(r));
    }

    template <const int R>
    __attribute__((always_inline)) static inline auto csrc(Register r) {
        asm volatile("csrc %0, %1" ::"i"(R), "r"(r));
    }

    __attribute__((always_inline)) static inline void idle() { asm volatile("wfi"); }

    __attribute__((always_inline)) static inline void ret() {
        if constexpr (MODE == Mode::SUPERVISOR)
            asm volatile("sret");
        else
            asm volatile("mret");
    }

    __attribute__((always_inline)) static inline unsigned int core() {
        unsigned int id;
        asm volatile("csrr %0, mhartid" : "=r"(id));
        return id;
    }

    __attribute__((naked)) static void init() { asm("ret"); }

    __attribute__((always_inline)) static inline void stack(void *ptr) { asm volatile("mv sp, %0" ::"r"(ptr)); }

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
            this->status = reinterpret_cast<uintptr_t>(0ULL | KERNEL2USER | KPIE);
            this->a0     = reinterpret_cast<uintptr_t>(a0);
        }

        __attribute__((naked)) static void load() {
            asm volatile(
                "ld t0, %c[status](sp)\n"
                "csrw %0, t0\n"
                "ld t0, %c[pc](sp)\n"
                "csrw %1, t0" ::"i"(KSTATUS),
                "i"(KEPC), [pc] "i"(OFFSET_OF(Context, pc)), [status] "i"(OFFSET_OF(Context, status)));
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
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [tp] "i"(OFFSET_OF(Context, tp)), [s0] "i"(OFFSET_OF(Context, s0)),
                  [s1] "i"(OFFSET_OF(Context, s1)), [a0] "i"(OFFSET_OF(Context, a0)), [s2] "i"(OFFSET_OF(Context, s2)),
                  [s3] "i"(OFFSET_OF(Context, s3)), [s4] "i"(OFFSET_OF(Context, s4)), [s5] "i"(OFFSET_OF(Context, s5)),
                  [s6] "i"(OFFSET_OF(Context, s6)), [s7] "i"(OFFSET_OF(Context, s7)), [s8] "i"(OFFSET_OF(Context, s8)),
                  [s9] "i"(OFFSET_OF(Context, s9)), [s10] "i"(OFFSET_OF(Context, s10)),
                  [s11] "i"(OFFSET_OF(Context, s11)));
            asm volatile("addi sp, sp, %0" ::"i"(sizeof(Context)));
            CPU::ret();
        }

        __attribute__((naked)) static void swtch(Context **previous, Context *next, Spin *lock) {
            asm volatile("addi sp, sp, %0" ::"i"(-sizeof(Context)));
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
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [tp] "i"(OFFSET_OF(Context, tp)), [s0] "i"(OFFSET_OF(Context, s0)),
                  [s1] "i"(OFFSET_OF(Context, s1)), [s2] "i"(OFFSET_OF(Context, s2)), [s3] "i"(OFFSET_OF(Context, s3)),
                  [s4] "i"(OFFSET_OF(Context, s4)), [s5] "i"(OFFSET_OF(Context, s5)), [s6] "i"(OFFSET_OF(Context, s6)),
                  [s7] "i"(OFFSET_OF(Context, s7)), [s8] "i"(OFFSET_OF(Context, s8)), [s9] "i"(OFFSET_OF(Context, s9)),
                  [s10] "i"(OFFSET_OF(Context, s10)), [s11] "i"(OFFSET_OF(Context, s11))
                : "memory");
            asm volatile(
                "csrr t0, mstatus\n"
                "and  t0, t0, %0\n"
                "or   t0, t0, %1\n"
                "sd   t0, %c[status](sp)\n"
                "sd   ra, %c[pc](sp)\n"
                "sd   sp, (%[prev])\n"
                "mv   sp, %[next]\n"
                :
                : "r"(~(KPIE | MPP)), "r"(KERNEL2KERNEL), [status] "i"(OFFSET_OF(Context, status)),
                  [pc] "i"(OFFSET_OF(Context, pc)), [prev] "r"(previous), [next] "r"(next)
                : "memory");
            lock->release();
            load();
        }

        __attribute__((always_inline)) static inline void push() {
            asm volatile("addi sp, sp, %0" ::"i"(-sizeof(Context)));
            asm volatile(
                "sd ra, %c[ra](sp)\n"
                "sd tp, %c[tp](sp)\n"
                "sd t0, %c[t0](sp)\n"
                "sd t1, %c[t1](sp)\n"
                "sd t2, %c[t2](sp)\n"
                "sd t3, %c[t3](sp)\n"
                "sd t4, %c[t4](sp)\n"
                "sd t5, %c[t5](sp)\n"
                "sd t6, %c[t6](sp)\n"
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [tp] "i"(OFFSET_OF(Context, tp)), [t0] "i"(OFFSET_OF(Context, t0)),
                  [t1] "i"(OFFSET_OF(Context, t1)), [t2] "i"(OFFSET_OF(Context, t2)), [t3] "i"(OFFSET_OF(Context, t3)),
                  [t4] "i"(OFFSET_OF(Context, t4)), [t5] "i"(OFFSET_OF(Context, t5)), [t6] "i"(OFFSET_OF(Context, t6))
                : "memory");

            asm volatile(
                "sd a0, %c[a0](sp)\n"
                "sd a1, %c[a1](sp)\n"
                "sd a2, %c[a2](sp)\n"
                "sd a3, %c[a3](sp)\n"
                "sd a4, %c[a4](sp)\n"
                "sd a5, %c[a5](sp)\n"
                "sd a6, %c[a6](sp)\n"
                "sd a7, %c[a7](sp)\n"
                :
                : [a0] "i"(OFFSET_OF(Context, a0)), [a1] "i"(OFFSET_OF(Context, a1)), [a2] "i"(OFFSET_OF(Context, a2)),
                  [a3] "i"(OFFSET_OF(Context, a3)), [a4] "i"(OFFSET_OF(Context, a4)), [a5] "i"(OFFSET_OF(Context, a5)),
                  [a6] "i"(OFFSET_OF(Context, a6)), [a7] "i"(OFFSET_OF(Context, a7))
                : "memory");
            asm volatile(
                "csrr t0, %0\n"
                "sd t0, %c[status](sp)\n"
                "csrr t0, %1\n"
                "sd t0, %c[pc](sp)" ::"i"(KSTATUS),
                "i"(KEPC), [status] "i"(OFFSET_OF(Context, status)), [pc] "i"(OFFSET_OF(Context, pc)));
        }

        __attribute__((naked)) static void pop() {
            asm volatile(
                "ld t0, %c[status](sp)\n"
                "csrw %0, t0\n"
                "ld t0, %c[pc](sp)\n"
                "csrw %1, t0" ::"i"(KSTATUS),
                "i"(KEPC), [pc] "i"(OFFSET_OF(Context, pc)), [status] "i"(OFFSET_OF(Context, status)));
            asm volatile(
                "ld ra, %c[ra](sp)\n"
                "ld tp, %c[tp](sp)\n"
                "ld t0, %c[t0](sp)\n"
                "ld t1, %c[t1](sp)\n"
                "ld t2, %c[t2](sp)\n"
                "ld t3, %c[t3](sp)\n"
                "ld t4, %c[t4](sp)\n"
                "ld t5, %c[t5](sp)\n"
                "ld t6, %c[t6](sp)\n"
                :
                : [ra] "i"(OFFSET_OF(Context, ra)), [tp] "i"(OFFSET_OF(Context, tp)), [t0] "i"(OFFSET_OF(Context, t0)),
                  [t1] "i"(OFFSET_OF(Context, t1)), [t2] "i"(OFFSET_OF(Context, t2)), [t3] "i"(OFFSET_OF(Context, t3)),
                  [t4] "i"(OFFSET_OF(Context, t4)), [t5] "i"(OFFSET_OF(Context, t5)), [t6] "i"(OFFSET_OF(Context, t6))
                : "memory");
            asm volatile(
                "ld a0, %c[a0](sp)\n"
                "ld a1, %c[a1](sp)\n"
                "ld a2, %c[a2](sp)\n"
                "ld a3, %c[a3](sp)\n"
                "ld a4, %c[a4](sp)\n"
                "ld a5, %c[a5](sp)\n"
                "ld a6, %c[a6](sp)\n"
                "ld a7, %c[a7](sp)\n"
                :
                : [a0] "i"(OFFSET_OF(Context, a0)), [a1] "i"(OFFSET_OF(Context, a1)), [a2] "i"(OFFSET_OF(Context, a2)),
                  [a3] "i"(OFFSET_OF(Context, a3)), [a4] "i"(OFFSET_OF(Context, a4)), [a5] "i"(OFFSET_OF(Context, a5)),
                  [a6] "i"(OFFSET_OF(Context, a6)), [a7] "i"(OFFSET_OF(Context, a7))
                : "memory");
            asm volatile("addi sp, sp, %0" ::"i"(sizeof(Context)));
            CPU::ret();
        }
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

        static void handler() {
            auto cause        = csrr<KCAUSE>();
            bool is_interrupt = cause >> (Machine::XLEN - 1);
            auto code         = (cause << 1) >> 1;

            if (is_interrupt) {
                interrupts[code]();
            } else {
                Kernel::exception();
            }
        }

        __attribute__((always_inline)) static inline void set(void (*p)()) {
            csrw<MTVEC>(reinterpret_cast<Register>(p));
        }
    };

    struct Interrupt {
        static void disable() { csrc<KSTATUS>(KERNEL_IRQ); }
        static void enable() { csrs<KSTATUS>(KERNEL_IRQ); }

        static void on() { enable(); }
        static bool off() {
            Register status = csrr<KSTATUS>();
            disable();
            return (status & 0x8) != 0;
        }

        struct Timer {
            static void enable() { csrs<KIE>(KTIE); }
            static void disable() { csrc<KIE>(KTIE); }
        };
    };
};
