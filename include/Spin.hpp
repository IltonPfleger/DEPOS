#pragma once
#include <CPU.hpp>
#include <IO/Debug.hpp>

class Spin {
    volatile bool locked         = !LOCKED;
    volatile bool interrupts     = true;
    static constexpr bool LOCKED = true;

   public:
    void acquire() {
        volatile auto i = CPU::Interrupt::off();
        while (__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE));
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        interrupts = i;
    }

    void release() {
        volatile auto i = interrupts;
        __atomic_thread_fence(__ATOMIC_RELEASE);
        __atomic_clear(&locked, __ATOMIC_RELEASE);
        if (i) CPU::Interrupt::on();
    }
};
