#pragma once

#include <Semaphore.hpp>
#include <Thread.hpp>
#include <utils/Observer.hpp>
#include <utils/Singleton.hpp>

template <typename Driver>
class NIC : public Observed<const unsigned char *, size_t>, private Observer<>, public Singleton<NIC<Driver>> {

  public:
    NIC() {
        m_running = true;
        Driver::init();
        Driver::instance()->attach(this);
        m_thread = new Thread(worker, this);
    };

    ~NIC() {
        m_running = false;
        m_semaphore.v();
        delete m_thread;
        Driver::instance()->detach(this);
    }

    static int worker(void *) {
        auto *nic = NIC<Driver>::s_instance;
        while (nic->m_running) {
            nic->m_semaphore.p();
            auto guard = Driver::instance()->receive();
            TraceIn();
            nic->notify(guard->data(), guard->length());
        }
        return 0;
    }

    void update() override { m_semaphore.v(); }

  private:
    volatile bool m_running;
    Thread *m_thread;
    Semaphore m_semaphore;
};
