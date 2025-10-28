#pragma once

class Spin {
    volatile bool locked     = false;
    volatile bool interrupts = false;

   public:
    void acquire() { while (__atomic_test_and_set(&locked, __ATOMIC_SEQ_CST)); }

    void release() { __atomic_clear(&locked, __ATOMIC_SEQ_CST); }

    void lock() {
        auto i = Machine::CPU::Interrupt::off();
        acquire();
        interrupts = i;
    }

    void unlock() {
        auto i = interrupts;
        release();
        if (i) Machine::CPU::Interrupt::on();
    }
};
