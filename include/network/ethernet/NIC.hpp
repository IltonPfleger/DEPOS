#pragma once

#include <Semaphore.hpp>
#include <Thread.hpp>
#include <utils/Observer.hpp>
#include <utils/Singleton.hpp>

template <typename Driver> class NIC : public Observed<const unsigned char *, size_t>, public Singleton<NIC<Driver>> {

  public:
    NIC() {
        m_running = true;
        Driver::init();
        m_thread = new Thread(worker, this);
    };

    ~NIC() {
        m_running = false;
        delete m_thread;
    }

    static int worker(void *) {
        auto *nic = NIC<Driver>::s_instance;
        while (nic->m_running) {
            auto guard = Driver::instance()->receive();
            nic->notify(guard->data(), guard->length());
        }
        return 0;
    }

  private:
    volatile bool m_running;
    Thread *m_thread;
};
