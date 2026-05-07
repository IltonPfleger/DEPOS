#pragma once

#include <Spin.hpp>
#include <Thread.hpp>

#include <utils/Lists.hpp>

namespace DEPOS {

class ConditionalVariable {

    struct Signal {
        Thread::Queue thread;
        void *value;
        size_t size;
    };

    typedef Node<Signal> Link;

  public:
    void wait(Spin *lock, void *value = nullptr, size_t size = 0) {
        Link link;
        Signal &signal = link.value();
        signal.value   = value;
        signal.size    = size;

        _waiters.insert(&link);
        Thread::sleep(&signal.thread, lock);
    }

    void signalize(size_t number = ~0ULL, void *value = 0, size_t size = 0) {
        while (number > 0) {
            Link *link = _waiters.remove();
            if (!link) break;

            Signal &signal = link->value();

            if (value && signal.value) {
                signal.size = (size > signal.size) ? signal.size : size;
                memcpy(signal.value, value, signal.size);
            } else {
                signal.size = 0;
            }

            signal.value = value;
            Thread::wakeup(&signal.thread);
            number--;
        }
    }

  private:
    LIFO<Link> _waiters;
};
} // namespace DEPOS
