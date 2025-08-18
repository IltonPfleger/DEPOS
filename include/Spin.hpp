#pragma once
#include <CPU.hpp>

class Spin {
    volatile int locked         = !LOCKED;
    static constexpr int LOCKED = 1;

   public:
    void acquire() { while (CPU::Atomic::tsl(locked)); }

    void release() { locked = !LOCKED; }

    void lock() {
        CPU::Interrupt::disable();
        acquire();
    }

    void unlock() {
        release();
        CPU::Interrupt::enable();
    }
};
