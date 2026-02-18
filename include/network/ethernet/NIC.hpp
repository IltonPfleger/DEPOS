#pragma once

#include <Semaphore.hpp>
#include <Thread.hpp>
#include <utils/Observer.hpp>

template <typename Driver> class NIC : public Observed<const unsigned char *, size_t>, private Observer<> {

  public:
    NIC() : semaphore(0) {
        Driver::init();
        Driver::instance()->attach(this);
        new Thread(worker, this);
    };

    static int worker(void *p) {
        NIC *nic = reinterpret_cast<NIC *>(p);
        while (1) {
            nic->semaphore.p();
            auto guard = Driver::instance()->receive();
            nic->notify(guard->data(), guard->length());
        }
    }

    void update() { semaphore.v(); }

  private:
    Semaphore semaphore;
};
