#pragma once
#include <Spin.hpp>
#include <Thread.hpp>

class Semaphore {
  public:
    Semaphore(unsigned int value = 1) : m_value(value) {}

    void p() {
        m_lock.lock();
        if (m_value-- < 1)
            Thread::sleep(m_waiting, m_lock);
        else
            m_lock.unlock();
    }

    void v() {
        m_lock.lock();
        if (m_value++ < 0) Thread::wakeup(m_waiting);
        m_lock.unlock();
    }

  private:
    int m_value;
    Thread::Queue m_waiting;
    Spin m_lock;
};
