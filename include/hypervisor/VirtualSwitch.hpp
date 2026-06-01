#pragma once

#include <Semaphore.hpp>
#include <Spin.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utility/collections/AtomicBoundedSimpleList.hpp>

namespace DEPOS {

template <typename Device> class VirtualSwitch : public Device::Observer, public Device::Observed {
  public:
    VirtualSwitch()
        : device_(Device::instance()) {
        device_->attach(this);
        running_ = true;
        thread_  = new Thread(entry, this);
    }

    ~VirtualSwitch() {
        running_ = false;
        delete thread_;
    }

    NetworkBuffer *alloc(size_t length) { return device_->alloc(length); }
    void release(const NetworkBuffer *buffer) { device_->release(buffer); }
    void retain(const NetworkBuffer &buffer) { device_->retain(buffer); }

    int send(NetworkBuffer *buffer) {
        size_t length = buffer->length();

        while (!spending_.insert(buffer))
            ;

        semaphore_.v();

        return length;
    }

    void update(const NetworkBuffer &buffer) {
        device_->retain(buffer);

        while (!rpending_.insert(&buffer))
            ;

        semaphore_.v();
    }

    static auto instance() {
        static VirtualSwitch instance;
        return &instance;
    }

  private:
    static void *entry(void *pointer) { return reinterpret_cast<VirtualSwitch *>(pointer)->worker(); }

    void *worker() {
        while (running_) {
            semaphore_.p();

            NetworkBuffer *snode;
            const NetworkBuffer *rnode;

            if (spending_.remove(&snode)) {
                this->notify(*snode);
                device_->send(snode);
            }

            if (rpending_.remove(&rnode)) {
                this->notify(*rnode);
                device_->release(const_cast<NetworkBuffer *>(rnode));
            }
        }
        return nullptr;
    }

  private:
    static constexpr size_t InternalQueueSize = 32;

    Device *device_;
    Thread *thread_;
    Spin lock_;

    AtomicBoundedSimpleList<NetworkBuffer, InternalQueueSize> spending_;
    AtomicBoundedSimpleList<const NetworkBuffer, InternalQueueSize> rpending_;

    volatile bool running_;
    Semaphore semaphore_;
};

} // namespace DEPOS
