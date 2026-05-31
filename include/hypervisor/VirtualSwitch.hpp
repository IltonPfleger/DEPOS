#pragma once

#include <Semaphore.hpp>
#include <Spin.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>

namespace DEPOS {

template <typename Device> class VirtualSwitch : public Device::Observer, public Device::Observed {

  public:
    static auto instance() {
        static VirtualSwitch instance;
        return &instance;
    }

  public:
    VirtualSwitch()
        : device_(Device::instance()) {

        for (size_t i = 0; i < InternalQueueSize; i++) {
            sfree_.insert(&snodes_[i]);
            rfree_.insert(&rnodes_[i]);
        }

        device_->attach(this);
        running_ = true;
        thread_  = new Thread(entry, this);
    }

    ~VirtualSwitch() {
        running_ = false;
        delete thread_;
    }

    VirtualSwitch(const VirtualSwitch &)            = delete;
    VirtualSwitch &operator=(const VirtualSwitch &) = delete;

    NetworkBuffer *alloc(size_t length) { return device_->alloc(length); }

    int send(NetworkBuffer *buffer) {
        size_t length = buffer->length();

        bool enabled = CPU::Interrupt::disable();
        lock_.acquire();
        auto *node = sfree_.remove();
        lock_.release();
        if (enabled) CPU::Interrupt::enable();

        if (!node) return 0;

        node->value(buffer);

        enabled = CPU::Interrupt::disable();
        lock_.acquire();
        spending_.insert(node);
        lock_.release();
        if (enabled) CPU::Interrupt::enable();

        semaphore_.v();

        return length;
    }

    void update(const NetworkBuffer &buffer) {
        bool enabled = CPU::Interrupt::disable();
        lock_.acquire();
        auto *node = rfree_.remove();
        lock_.release();
        if (enabled) CPU::Interrupt::enable();

        if (!node) return;

        device_->retain(buffer);
        node->value(&buffer);

        enabled = CPU::Interrupt::disable();
        lock_.acquire();
        rpending_.insert(node);
        lock_.release();
        if (enabled) CPU::Interrupt::enable();

        semaphore_.v();
    }

  private:
    static void *entry(void *pointer) { return reinterpret_cast<VirtualSwitch *>(pointer)->worker(); }

    void *worker() {
        while (running_) {
            semaphore_.p();
            CPU::Interrupt::disable();
            lock_.acquire();
            SendNode *snode     = spending_.remove();
            ReceivedNode *rnode = rpending_.remove();
            lock_.release();
            CPU::Interrupt::enable();

            if (snode) {
                this->notify(*snode->value());
                device_->send(snode->value());
                CPU::Interrupt::disable();
                lock_.acquire();
                sfree_.insert(snode);
                lock_.release();
                CPU::Interrupt::enable();
            }

            if (rnode) {
                this->notify(*rnode->value());
                device_->release(const_cast<NetworkBuffer *>(rnode->value()));
                CPU::Interrupt::disable();
                lock_.acquire();
                rfree_.insert(rnode);
                lock_.release();
                CPU::Interrupt::enable();
            }
        }
        return nullptr;
    }

  private:
    static constexpr size_t InternalQueueSize = 16;

    using SendNode     = collections::Node<NetworkBuffer *>;
    using SendList     = collections::FIFO<SendNode>;
    using ReceivedNode = collections::Node<const NetworkBuffer *>;
    using ReceivedList = collections::FIFO<ReceivedNode>;

    Device *device_;
    Thread *thread_;
    Spin lock_;

    SendList sfree_;
    SendList spending_;
    SendNode snodes_[InternalQueueSize];

    ReceivedList rfree_;
    ReceivedList rpending_;
    ReceivedNode rnodes_[InternalQueueSize];

    volatile bool running_;
    Semaphore semaphore_;
};

} // namespace DEPOS
