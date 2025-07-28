#pragma once
#include <CPU.hpp>

class Spin {
    int _locked;

   public:
    static constexpr int LOCKED = 1;

    Spin(int locked = !LOCKED) : _locked(locked) {}
    void acquire() { CPU::Atomic::lock(&_locked); }
    void release() { CPU::Atomic::unlock(&_locked); }

    void lock() {
        CPU::Interrupt::disable();
        acquire();
    }

    void unlock() {
        release();
        CPU::Interrupt::enable();
    }
};
