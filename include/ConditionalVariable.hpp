#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <utility/collections/FIFO.hpp>

namespace DEPOS {

class ConditionalVariable {
  private:
    struct Message {
        Thread::Queue thread;
        void *value;
        size_t size;
    };

    typedef collections::Node<Message> Link;

  public:
    ConditionalVariable() = default;

    size_t wait(Spin *lock, void *value = nullptr, size_t size = 0) {
        Link link;
        Message &message = link.value();
        message.value    = value;
        message.size     = size;

        waiters_.insert(&link);
        waiting_++;
        Thread::sleep(&message.thread, lock);

        return message.size;
    }

    bool send(const void *value = 0, size_t size = 0) {
        Link *link = waiters_.remove();

        if (!link) return false;

        Message &message = link->value();

        if (value && message.value) {
            message.size = (size > message.size) ? message.size : size;
            memcpy(message.value, value, message.size);
        } else {
            message.size = 0;
        }

        waiting_--;
        Thread::wakeup(&message.thread);

        return true;
    }

    size_t count() const { return waiting_; }

  private:
    collections::FIFO<Link> waiters_{};
    size_t waiting_{};
};

} // namespace DEPOS
