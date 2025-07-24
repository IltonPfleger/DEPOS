#pragma once
#include <CPU.hpp>

struct Spin {
    int value = 0;

   public:
    void lock() { CPU::Atomic::lock(&value); }
    void unlock() { CPU::Atomic::unlock(&value); }
};
