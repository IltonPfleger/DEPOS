class Context {
  public:
    uint64_t ra;
    uint64_t gp;
    uint64_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t t0, t1, t2, t3, t4, t5, t6;
    uint64_t status;
    uint64_t pc;

    Context(int (*entry)(void *), void *a0, void (*exit)()) {
        this->ra = reinterpret_cast<uint64_t>(exit);
        this->pc = reinterpret_cast<uint64_t>(entry);
        this->status = static_cast<uint64_t>(
            KernelMode::ME2ME | static_cast<uint64_t>(KernelMode::PIRQE));
        this->a0 = reinterpret_cast<uint64_t>(a0);
    }

    template <typename T = KernelMode> void load() {
        asm volatile(
            "mv sp, %[sp]\n"
            "ld ra,	 %[ra](sp)\n"
            "ld gp,  %[gp](sp)\n"
            "ld s0,  %[s0](sp)\n"
            "ld a0,  %[a0](sp)\n"
            "ld s1,  %[s1](sp)\n"
            "ld s2,  %[s2](sp)\n"
            "ld s3,  %[s3](sp)\n"
            "ld s4,  %[s4](sp)\n"
            "ld s5,  %[s5](sp)\n"
            "ld s6,  %[s6](sp)\n"
            "ld s7,  %[s7](sp)\n"
            "ld s8,  %[s8](sp)\n"
            "ld s9,  %[s9](sp)\n"
            "ld s10, %[s10](sp)\n"
            "ld s11, %[s11](sp)\n"
            "ld t0,  %[status](sp)\n"
            "ld t1,  %[pc](sp)\n"
            "csrw    %[estatus], t0\n"
            "csrw    %[epc], t1\n"
            "addi sp, sp, %[size]\n"
            :
            : [sp] "r"(this), [ra] "i"(offsetof(Context, ra)),
              [gp] "i"(offsetof(Context, gp)), [s0] "i"(offsetof(Context, s0)),
              [s1] "i"(offsetof(Context, s1)), [a0] "i"(offsetof(Context, a0)),
              [s2] "i"(offsetof(Context, s2)), [s3] "i"(offsetof(Context, s3)),
              [s4] "i"(offsetof(Context, s4)), [s5] "i"(offsetof(Context, s5)),
              [s6] "i"(offsetof(Context, s6)), [s7] "i"(offsetof(Context, s7)),
              [s8] "i"(offsetof(Context, s8)), [s9] "i"(offsetof(Context, s9)),
              [s10] "i"(offsetof(Context, s10)),
              [s11] "i"(offsetof(Context, s11)), [size] "i"(sizeof(Context)),
              [estatus] "i"(T::STATUS), [epc] "i"(T::EPC),
              [pc] "i"(offsetof(Context, pc)),
              [status] "i"(offsetof(Context, status)));
        T::ret();
        __builtin_unreachable();
    }

    template <typename T = KernelMode>
    __attribute__((naked)) static void swtch(Context **previous,
                                             Context *next) {
        Context *saved;
        asm volatile(
            "addi sp, sp, %[size]\n"
            "sd   ra,     %[ra](sp)\n"
            "sd   gp,     %[gp](sp)\n"
            "sd   s0,     %[s0](sp)\n"
            "sd   s1,     %[s1](sp)\n"
            "sd   s2,     %[s2](sp)\n"
            "sd   s3,     %[s3](sp)\n"
            "sd   s4,     %[s4](sp)\n"
            "sd   s5,     %[s5](sp)\n"
            "sd   s6,     %[s6](sp)\n"
            "sd   s7,     %[s7](sp)\n"
            "sd   s8,     %[s8](sp)\n"
            "sd   s9,     %[s9](sp)\n"
            "sd   s10,    %[s10](sp)\n"
            "sd   s11,    %[s11](sp)\n"
            "csrr t0,     %[statusr]\n"
            "sd   t0,     %[statuso](sp)\n"
            "sd   ra,     %[pc](sp)\n"
            "mv   %[saved], sp\n"
            : [saved] "=r"(saved)
            : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)),
              [s0] "i"(offsetof(Context, s0)), [s1] "i"(offsetof(Context, s1)),
              [s2] "i"(offsetof(Context, s2)), [s3] "i"(offsetof(Context, s3)),
              [s4] "i"(offsetof(Context, s4)), [s5] "i"(offsetof(Context, s5)),
              [s6] "i"(offsetof(Context, s6)), [s7] "i"(offsetof(Context, s7)),
              [s8] "i"(offsetof(Context, s8)), [s9] "i"(offsetof(Context, s9)),
              [s10] "i"(offsetof(Context, s10)),
              [s11] "i"(offsetof(Context, s11)), [size] "i"(-sizeof(Context)),
              [statusr] "i"(T::STATUS),
              [statuso] "i"(offsetof(Context, status)),
              [pc] "i"(offsetof(Context, pc))
            : "memory");
        saved->status |= (T::ME2ME);
        *previous = saved;
        next->load();
    }

    template <typename T = KernelMode>
    __attribute__((always_inline)) static inline Context *push() {
        asm volatile(
            "addi sp, sp, %[size]\n"
            "sd ra, %[ra](sp)\n"
            "sd gp, %[gp](sp)\n"
            "sd t0, %[t0](sp)\n"
            "sd t1, %[t1](sp)\n"
            "sd t2, %[t2](sp)\n"
            "sd t3, %[t3](sp)\n"
            "sd t4, %[t4](sp)\n"
            "sd t5, %[t5](sp)\n"
            "sd t6, %[t6](sp)\n"
            "sd a0, %[a0](sp)\n"
            "sd a1, %[a1](sp)\n"
            "sd a2, %[a2](sp)\n"
            "sd a3, %[a3](sp)\n"
            "sd a4, %[a4](sp)\n"
            "sd a5, %[a5](sp)\n"
            "sd a6, %[a6](sp)\n"
            "sd a7, %[a7](sp)\n"
            :
            : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)),
              [t0] "i"(offsetof(Context, t0)), [t1] "i"(offsetof(Context, t1)),
              [t2] "i"(offsetof(Context, t2)), [t3] "i"(offsetof(Context, t3)),
              [t4] "i"(offsetof(Context, t4)), [t5] "i"(offsetof(Context, t5)),
              [t6] "i"(offsetof(Context, t6)), [a0] "i"(offsetof(Context, a0)),
              [a1] "i"(offsetof(Context, a1)), [a2] "i"(offsetof(Context, a2)),
              [a3] "i"(offsetof(Context, a3)), [a4] "i"(offsetof(Context, a4)),
              [a5] "i"(offsetof(Context, a5)), [a6] "i"(offsetof(Context, a6)),
              [a7] "i"(offsetof(Context, a7)), [size] "i"(-sizeof(Context))
            : "memory");

        asm volatile("csrr t0, %0\n"
                     "sd t0, %c[status](sp)\n"
                     "csrr t0, %1\n"
                     "sd t0, %c[pc](sp)" ::"i"(T::STATUS),
                     "i"(T::EPC), [status] "i"(offsetof(Context, status)),
                     [pc] "i"(offsetof(Context, pc)));
        register Context *sp asm("sp");
        return sp;
    }

    template <typename T = KernelMode>
    __attribute__((naked)) static void pop() {
        asm volatile("ld t0, %[statuso](sp)\n"
                     "csrw %[statusr], t0\n"
                     "ld t0, %[pc](sp)\n"
                     "csrw %[epcr], t0" ::[statusr] "i"(T::STATUS),
                     [epcr] "i"(T::EPC), [pc] "i"(offsetof(Context, pc)),
                     [statuso] "i"(offsetof(Context, status)));
        asm volatile(
            "ld ra, %[ra](sp)\n"
            "ld gp, %[gp](sp)\n"
            "ld t0, %[t0](sp)\n"
            "ld t1, %[t1](sp)\n"
            "ld t2, %[t2](sp)\n"
            "ld t3, %[t3](sp)\n"
            "ld t4, %[t4](sp)\n"
            "ld t5, %[t5](sp)\n"
            "ld t6, %[t6](sp)\n"
            "ld a0, %[a0](sp)\n"
            "ld a1, %[a1](sp)\n"
            "ld a2, %[a2](sp)\n"
            "ld a3, %[a3](sp)\n"
            "ld a4, %[a4](sp)\n"
            "ld a5, %[a5](sp)\n"
            "ld a6, %[a6](sp)\n"
            "ld a7, %[a7](sp)\n"
            "addi sp, sp, %[size]\n"
            :
            : [ra] "i"(offsetof(Context, ra)), [gp] "i"(offsetof(Context, gp)),
              [t0] "i"(offsetof(Context, t0)), [t1] "i"(offsetof(Context, t1)),
              [t2] "i"(offsetof(Context, t2)), [t3] "i"(offsetof(Context, t3)),
              [t4] "i"(offsetof(Context, t4)), [t5] "i"(offsetof(Context, t5)),
              [t6] "i"(offsetof(Context, t6)), [a0] "i"(offsetof(Context, a0)),
              [a1] "i"(offsetof(Context, a1)), [a2] "i"(offsetof(Context, a2)),
              [a3] "i"(offsetof(Context, a3)), [a4] "i"(offsetof(Context, a4)),
              [a5] "i"(offsetof(Context, a5)), [a6] "i"(offsetof(Context, a6)),
              [a7] "i"(offsetof(Context, a7)), [size] "i"(sizeof(Context))
            : "memory");
        T::ret();
        __builtin_unreachable();
    }
};
