#pragma once

#include <Semaphore.hpp>
#include <Spin.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utility/collections/FIFO.hpp>

namespace DEPOS {

template <typename Device> class VirtualSwitch : public Device::Observer, public Device::Observed {
  public:
    VirtualSwitch()
        : device_(Device::instance()) {
        device_->attach(this);
        running_ = true;
        thread_  = new Thread(entry, this, Thread::Criterion::SYSTEM);
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
        spending_.insert(buffer->node());
        semaphore_.v();
        return length;
    }

    void update(const NetworkBuffer &buffer) {
        device_->retain(&buffer);
        rpending_.insert(buffer.node());
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

            NetworkBuffer::Node *snode = spending_.remove();
            NetworkBuffer::Node *rnode = rpending_.remove();

            if (snode) {
                this->notify(*snode->value());
                device_->send(snode->value());
            }

            if (rnode) {
                this->notify(*rnode->value());
                device_->release(rnode->value());
            }
        }
        return nullptr;
    }

  private:
    static constexpr size_t InternalQueueSize = 32;

    Device *device_;
    Thread *thread_;

    collections::FIFO<NetworkBuffer::Node, true> spending_;
    collections::FIFO<NetworkBuffer::Node, true> rpending_;

    volatile bool running_;
    Semaphore semaphore_;
};

} // namespace DEPOS
