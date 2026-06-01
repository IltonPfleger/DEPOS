#ifndef __DEPOS_NETWORK_VAMPIRE__
#define __DEPOS_NETWORK_VAMPIRE__

#include <Semaphore.hpp>
#include <Thread.hpp>
#include <architecture/CPU.hpp>
#include <architecture/Timer.hpp>
#include <libraries/libc/string.h>
#include <network/NetworkBuffer.hpp>

namespace DEPOS {

template <typename Device> class NetworkVampire : Device::Observer {
  public:
    NetworkVampire(Thread::Criterion criterion)
        : device_(Device::instance()),
          buffer_(nullptr),
          running_(true) {
        device_->attach(this);
        thread_ = new Thread(entry, this, criterion);
    }

    ~NetworkVampire() {
        device_->detach(this);
        running_ = false;
        semaphore_.v();
        delete thread_;
        if (buffer_) {
            device_->release(buffer_);
        }
    }

    size_t random() {
        static uintmax_t state = Timer::us();
        state                  = state * 1103515245 + 12345;
        return (state / 65536) % 100;
    }

    void update(const NetworkBuffer &buffer) override {
        if (random() >= 50) return;

        device_->retain(buffer);

        auto &raw = const_cast<const NetworkBuffer *&>(buffer_);
        if (!CPU::Atomic::cas<const NetworkBuffer *>(raw, nullptr, &buffer)) {
            device_->release(&buffer);
        }

        semaphore_.v();
    }

  private:
    static void *entry(void *pointer) { return reinterpret_cast<NetworkVampire *>(pointer)->worker(); }

    void *worker() {
        while (running_) {
            semaphore_.p();

            if (buffer_ == nullptr) continue;

            NetworkBuffer *clone = device_->alloc(buffer_->length());
            clone->shrink(clone->offset());
            clone->rewind(clone->offset());
            memcpy(clone->start(), buffer_->start(), buffer_->length());
            device_->send(clone);

            const NetworkBuffer *release = buffer_;
            buffer_                      = nullptr;
            device_->release(release);
        }
        return nullptr;
    }

  private:
    Device *device_;
    const NetworkBuffer *volatile buffer_;
    Thread *thread_;
    Semaphore semaphore_;
    volatile bool running_;
};

} // namespace DEPOS

#endif
