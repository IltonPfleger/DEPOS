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
    ConditionalVariable() = default;

    size_t wait(Spin *lock, void *value = nullptr, size_t size = 0) {
        Link link;
        Signal &signal = link.value();
        signal.value   = value;
        signal.size    = size;

        _waiters.insert(&link);
        _waiting++;
        Thread::sleep(&signal.thread, lock);

        return signal.size;
    }

    void signalize(const void *value = 0, size_t size = 0) {
        Link *link = _waiters.remove();
        if (!link) return;

        Signal &signal = link->value();

        if (value && signal.value) {
            signal.size = (size > signal.size) ? signal.size : size;
            memcpy(signal.value, value, signal.size);
        } else {
            signal.size = 0;
        }

        _waiting--;
        Thread::wakeup(&signal.thread);
    }

    size_t count() const { return _waiting; }

  private:
    LIFO<Link> _waiters{};
    size_t _waiting{};
};

} // namespace DEPOS
