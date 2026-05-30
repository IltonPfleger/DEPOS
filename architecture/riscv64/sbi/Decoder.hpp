#ifndef __DEPOS_RISCV64_SBI_DECODER__
#define __DEPOS_RISCV64_SBI_DECODER__

namespace DEPOS::riscv64::sbi {

class Decoder {
  public:
    struct LD {};
    struct SD {};

    static uint8_t rd(LD, uintptr_t pc) {
        const uint16_t instruction16 = *reinterpret_cast<const uint16_t *>(pc);

        if ((instruction16 & 0x3) != 0x3) {
            return 8 + ((instruction16 >> 2) & 0x7);
        }

        uint32_t instruction = *reinterpret_cast<const uint32_t *>(pc);
        uint8_t opcode       = instruction & 0x7F;
        uint8_t funct3       = (instruction >> 12) & 0x7;

        if (opcode == 0x03 && funct3 == 0x3) {
            return (instruction >> 7) & 0x1F;
        }

        return 0;
    }

    static uint8_t rs2(SD, uintptr_t pc) {
        const uint16_t instruction16 = *reinterpret_cast<const uint16_t *>(pc);

        if ((instruction16 & 0x3) != 0x3) {
            uint8_t funct3 = (instruction16 >> 13) & 0x7;
            if (funct3 == 0x7) return 8 + ((instruction16 >> 2) & 0x7);
            return 0;
        }

        uint32_t instruction = *reinterpret_cast<const uint32_t *>(pc);
        uint8_t opcode       = instruction & 0x7F;
        uint8_t funct3       = (instruction >> 12) & 0x7;
        if (opcode == 0x23 && funct3 == 0x3) return (instruction >> 20) & 0x1F;
        return 0;
    }
};

} // namespace DEPOS::riscv64::sbi

#endif
