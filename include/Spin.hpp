#pragma once
#include <CPU.hpp>

struct Spin {
    int value = !LOCKED;

   public:
    static constexpr int LOCKED = 1;
    void lock() { CPU::Atomic::lock(&value); }
    void unlock() { CPU::Atomic::unlock(&value); }
};
