#pragma once
#include <Spin.hpp>
#include <Thread.hpp>

class Semaphore {
  public:
    Semaphore(int value = 1) : m_value(value) {}

    void p() {
        CPU::Interruptions::disable();
        m_spin.acquire();
        if (m_value-- < 1)
            Thread::sleep(&m_waiting, &m_spin);
        else
            m_spin.release();
        CPU::Interruptions::enable();
    }

    void v() {
        CPU::Interruptions::disable();
        m_spin.acquire();
        if (m_value++ < 0) Thread::wakeup(&m_waiting);
        m_spin.release();
        CPU::Interruptions::enable();
    }

  private:
    int m_value;
    Thread::Queue m_waiting;
    Spin m_spin;
};
