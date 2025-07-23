#pragma once
#include <CPU.hpp>

class Spin {
    int _lock = 0;

   public:
    void lock() { CPU::Atomic::lock(&_lock); }
    void unlock() { CPU::Atomic::unlock(&_lock); }
};
