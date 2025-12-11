#pragma once
#include <machine/Machine.hpp>

class Spin {
  public:
    Spin() : m_locked(false), m_interrupts(false) {}

    void acquire() {
        while (__atomic_test_and_set(&m_locked, __ATOMIC_SEQ_CST))
            ;
    }

    void release() { __atomic_clear(&m_locked, __ATOMIC_SEQ_CST); }

    void lock() {
        auto i = CPU::Interruptions::off();
        acquire();
        m_interrupts = i;
    }

    void unlock() {
        auto i = m_interrupts;
        release();
        if (i)
            CPU::Interruptions::on();
    }

  private:
    volatile bool m_locked;
    volatile bool m_interrupts;
};
