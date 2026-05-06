#pragma once

#include <Spin.hpp>
#include <Thread.hpp>

#include <utils/Lists.hpp>

namespace DEPOS {

class ConditionalVariable {

    struct Signal {
        Thread::Queue thread;
        void *value;
    };

    typedef Node<Signal> Link;

  public:
    void *wait(Spin *lock) {
        Link link;
        Signal &signal = link.value();

        _waiters.insert(&link);
        Thread::sleep(&signal.thread, lock);

        return signal.value;
    }

    void signalize(size_t number = ~0ULL, void *value = 0) {
        while (number > 0) {
            Link *link = _waiters.remove();
            if (!link) break;
            Signal &signal = link->value();
            signal.value   = value;
            Thread::wakeup(&signal.thread);
            number--;
        }
    }

  private:
    LIFO<Link> _waiters;
};
} // namespace DEPOS
