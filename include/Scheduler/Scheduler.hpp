#pragma once
#include <Scheduler/Lists.hpp>

template <typename T>
struct RR {
    static constexpr bool Timed = false;
};

template <typename T>
struct RateMonotonic {
    static constexpr bool Timed = true;
};

template <typename T>
struct Scheduler : POFO<T *> {
    using Queue = POFO<T *>;
    using Queue::empty;
    using Queue::insert;
    using Queue::next;
    using Queue::remove;

    T *chose() {
        auto *element = next();
        return element->value;
    }
};
