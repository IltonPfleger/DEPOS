#pragma once

class Driver {
  public:
    static auto &Reg64(unsigned long base, unsigned int offset) {
        return *reinterpret_cast<volatile unsigned long *>(base + offset);
    }

    static auto &Reg32(unsigned long base, unsigned int offset) {
        return *reinterpret_cast<volatile unsigned int *>(base + offset);
    }

    static auto &Reg16(unsigned long base, unsigned int offset) {
        return *reinterpret_cast<volatile unsigned short *>(base + offset);
    }

    static auto &Reg8(unsigned long base, unsigned int offset) {
        return *reinterpret_cast<volatile unsigned char *>(base + offset);
    }
};
