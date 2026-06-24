#pragma once

#include <Mutex.hpp>
#include <Spin.hpp>
#include <Thread.hpp>
#include <utility/collections/FIFO.hpp>

namespace QUARK {

class ConditionalVariable {
  private:
    struct Message {
        Thread::List thread;
        void *value;
        size_t size;
    };

    typedef collections::Node<Message> Node;

  public:
    constexpr ConditionalVariable()
        : waiting_(0) {};

    size_t wait(Mutex &external, void *value = nullptr, size_t size = 0) {
        Node node;
        Message &message = node.value;

        message.value = value;
        message.size  = size;

        bool enabled = CPU::IRQ::disable();
        lock_.acquire();
        external.release();

        waiters_.insert(&node);
        waiting_++;
        Thread::sleep(&message.thread, &lock_);

        if (enabled) CPU::IRQ::enable();

        return message.size;
    }

    bool send(const void *value = 0, size_t size = 0) {
        bool enabled = CPU::IRQ::disable();
        lock_.acquire();

        Node *node = nullptr;
        if (waiting_) {
            node = waiters_.remove();
            waiting_--;
        }

        lock_.release();
        if (enabled) CPU::IRQ::enable();

        if (!node) return false;

        Message &message = node->value;

        if (value && message.value) {
            message.size = (size > message.size) ? message.size : size;
            memcpy(message.value, value, message.size);
        } else {
            message.size = 0;
        }

        Thread::wakeup(&message.thread);

        return true;
    }

    size_t count() const { return waiting_; }

  private:
    collections::FIFO<Node> waiters_;
    Spin lock_;
    size_t waiting_;
};

} // namespace QUARK
