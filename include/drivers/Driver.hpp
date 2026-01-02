#pragma once

// template <typename T>
// concept DriverInterface = requires(T a) {
//     { a.write(...) } -> int;
//     { a.read(...) } -> int;
// };

class Driver {
  public:
    static auto &Reg64(unsigned long base, unsigned int offset) {
        return *reinterpret_cast<volatile unsigned long *>(base + offset);
    }

    static auto &Reg32(unsigned long base, unsigned int offset) {
        return *reinterpret_cast<volatile unsigned int *>(base + offset);
    }
};
