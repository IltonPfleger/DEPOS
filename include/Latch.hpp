#pragma once

#include <Spin.hpp>
#include <Thread.hpp>

namespace DEPOS {

class Latch {
  public:
    Latch()
        : m_waiting_counter(0) {}

    void *p() {
        m_spin.acquire();
        m_waiting_counter = m_waiting_counter + 1;
        Thread::sleep(&m_waiting_list, &m_spin);
        void *message = const_cast<void *>(m_message);
        CPU::Atomic::fdec(m_done_counter);
        return message;
    }

    int v(void *message) {
        m_spin.acquire();
        int response   = m_waiting_counter;
        m_message      = message;
        m_done_counter = m_waiting_counter;
        while (m_waiting_counter) {
            Thread::wakeup(&m_waiting_list);
            m_waiting_counter = m_waiting_counter - 1;
        }
        while (m_done_counter)
            ;
        m_spin.release();
        return response;
    }

  private:
    volatile int m_waiting_counter;
    volatile int m_done_counter;
    volatile void *m_message;
    Thread::Queue m_waiting_list;
    Spin m_spin;
};

} // namespace DEPOS
