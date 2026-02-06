#pragma once
#include <Spin.hpp>
#include <Thread.hpp>

class Semaphore {
  public:
    Semaphore(unsigned int value = 1) : m_value(value) {}

    void p() {
        m_spin.acquire();
        if (m_value-- < 1)
            Thread::sleep(&m_waiting, &m_spin);
        else
            m_spin.release();
    }

    void v() {
        m_spin.acquire();
        if (m_value++ < 0) Thread::wakeup(&m_waiting);
        m_spin.release();
    }

  private:
    int m_value;
    Thread::Queue m_waiting;
    Spin m_spin;
};
