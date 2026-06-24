#ifndef __QUARK_NETWORK_VAMPIRE__
#define __QUARK_NETWORK_VAMPIRE__

#include <Mutex.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>
#include <architecture/CPU.hpp>
#include <architecture/Timer.hpp>
#include <libraries/libc/string.h>
#include <network/NetworkBuffer.hpp>

namespace QUARK {

template <typename DEVICE> class NetworkVampire : DEVICE::Observer {
  public:
    NetworkVampire()
        : device_(DEVICE::instance()) {
        device_->attach(this);
        new Thread(worker, this);
    }

    size_t random() {
        static size_t state = 0;
        state               = state * 6364136223846793005ULL + 1442695040888963407ULL;
        return (state / 65536) % 100;
    }

    void update(const NetworkBuffer *buffer) override {
        if (random() >= 80) {
            lock_.acquire();

            memcpy(buffer_, buffer->start(), buffer->length());
            length_ = buffer->length();

            lock_.release();
            p_.v();
        }
    }

    static void *worker(void *pointer) {
        NetworkVampire *self = reinterpret_cast<NetworkVampire *>(pointer);
        while (1) {
            self->p_.p();

            int k = self->random() % 5;
            for (int i = 0; i < k; i++) {
                self->lock_.acquire();
                NetworkBuffer *clone = self->device_->alloc(self->length_);
                memcpy(clone->start(), self->buffer_, self->length_);
                self->lock_.release();
                self->device_->send(clone);
            }
        }
        return nullptr;
    }

  private:
    DEVICE *device_;
    Mutex lock_;
    Semaphore p_;
    unsigned char buffer_[1522];
    size_t length_;
};

} // namespace QUARK

#endif
