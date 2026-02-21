#pragma once

#include <Spin.hpp>
#include <Thread.hpp>

class Latch {
  public:
    Latch() : m_count(0) {}

    void p() {
        m_spin.acquire();
        m_count++;
        Thread::sleep(&m_waiting, &m_spin);
    }

    void v() {
        m_spin.acquire();

        while (m_count > 0) {
            Thread::wakeup(&m_waiting);
            m_count--;
        }

        m_spin.release();
    }

  private:
    int m_count;
    Thread::Queue m_waiting;
    Spin m_spin;
};
