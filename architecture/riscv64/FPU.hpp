#ifndef __DEPOS_RISCV64_FPU__
#define __DEPOS_RISCV64_FPU__

namespace DEPOS {

class FPU {
    struct Frame {
        uint64_t f[32];
        uint64_t fcsr;
    };

  public:
    static void enable(ContextFrame *context) {
        context->status &= ~FS_MASK;
        context->status |= FS_INITIAL;
    }

    template <typename T> __attribute__((always_inline)) static inline void save() {
        asm("csrr t0, %0; srli t0, t0, 13; andi t0, t0, 3; beqz t0, 1f" ::"i"(T::STATUS));
        asm("addi sp, sp, %0" ::"i"(-sizeof(Frame)));
        asm("fsd f0,  0(sp)\n"
            "fsd f1,  8(sp)\n"
            "fsd f2,  16(sp)\n"
            "fsd f3,  24(sp)\n"
            "fsd f4,  32(sp)\n"
            "fsd f5,  40(sp)\n"
            "fsd f6,  48(sp)\n"
            "fsd f7,  56(sp)\n"
            "fsd f8,  64(sp)\n"
            "fsd f9,  72(sp)\n"
            "fsd f10, 80(sp)\n"
            "fsd f11, 88(sp)\n"
            "fsd f12, 96(sp)\n"
            "fsd f13, 104(sp)\n"
            "fsd f14, 112(sp)\n"
            "fsd f15, 120(sp)\n"
            "fsd f16, 128(sp)\n"
            "fsd f17, 136(sp)\n"
            "fsd f18, 144(sp)\n"
            "fsd f19, 152(sp)\n"
            "fsd f20, 160(sp)\n"
            "fsd f21, 168(sp)\n"
            "fsd f22, 176(sp)\n"
            "fsd f23, 184(sp)\n"
            "fsd f24, 192(sp)\n"
            "fsd f25, 200(sp)\n"
            "fsd f26, 208(sp)\n"
            "fsd f27, 216(sp)\n"
            "fsd f28, 224(sp)\n"
            "fsd f29, 232(sp)\n"
            "fsd f30, 240(sp)\n"
            "fsd f31, 248(sp)\n"
            "csrr t0, fcsr\n"
            "sd   t0, 256(sp)");
        asm("1:");
    }

    template <typename T> __attribute__((always_inline)) static inline void load() {
        asm("csrr t0, %0; srli t0, t0, 13; andi t0, t0, 3; beqz t0, 1f" ::"i"(T::STATUS));
        asm("fld f0,  0(sp)\n"
            "fld f1,  8(sp)\n"
            "fld f2,  16(sp)\n"
            "fld f3,  24(sp)\n"
            "fld f4,  32(sp)\n"
            "fld f5,  40(sp)\n"
            "fld f6,  48(sp)\n"
            "fld f7,  56(sp)\n"
            "fld f8,  64(sp)\n"
            "fld f9,  72(sp)\n"
            "fld f10, 80(sp)\n"
            "fld f11, 88(sp)\n"
            "fld f12, 96(sp)\n"
            "fld f13, 104(sp)\n"
            "fld f14, 112(sp)\n"
            "fld f15, 120(sp)\n"
            "fld f16, 128(sp)\n"
            "fld f17, 136(sp)\n"
            "fld f18, 144(sp)\n"
            "fld f19, 152(sp)\n"
            "fld f20, 160(sp)\n"
            "fld f21, 168(sp)\n"
            "fld f22, 176(sp)\n"
            "fld f23, 184(sp)\n"
            "fld f24, 192(sp)\n"
            "fld f25, 200(sp)\n"
            "fld f26, 208(sp)\n"
            "fld f27, 216(sp)\n"
            "fld f28, 224(sp)\n"
            "fld f29, 232(sp)\n"
            "fld f30, 240(sp)\n"
            "fld f31, 248(sp)\n"
            "ld  t0, 256(sp)\n"
            "csrw fcsr, t0");
        asm("addi sp, sp, %0" ::"i"(sizeof(Frame)));
        asm("1:");
    }

  private:
    static constexpr uint64_t FS_SHIFT   = 13;
    static constexpr uint64_t FS_MASK    = (3ULL << FS_SHIFT);
    static constexpr uint64_t FS_OFF     = (0ULL << FS_SHIFT);
    static constexpr uint64_t FS_INITIAL = (1ULL << FS_SHIFT);
    static constexpr uint64_t FS_CLEAN   = (2ULL << FS_SHIFT);
    static constexpr uint64_t FS_DIRTY   = (3ULL << FS_SHIFT);
};

} // namespace DEPOS

#endif
