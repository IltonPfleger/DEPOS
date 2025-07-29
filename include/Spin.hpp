#pragma once
#include <CPU.hpp>

struct Spin {
    volatile int _locked = !LOCKED;

    static constexpr int LOCKED = 1;
    void acquire() { while (CPU::Atomic::tsl(&_locked)); }
    void release() { _locked = !LOCKED; }

    void lock() {
        CPU::Interrupt::disable();
        acquire();
    }

    void unlock() {
        release();
        CPU::Interrupt::enable();
    }
};
