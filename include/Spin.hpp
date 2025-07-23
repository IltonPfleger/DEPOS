#pragma once
#include <CPU.hpp>

class Spin {
   public:
    int _lock = 0;
    void lock() { CPU::Atomic::lock(&_lock); }
    void unlock() { CPU::Atomic::unlock(&_lock); }
};
