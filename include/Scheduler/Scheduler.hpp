#pragma once
#include <Scheduler/Lists.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <Types.hpp>

template <typename T>
struct RR {
    using Rank                  = uintptr_t;
    static constexpr bool Timed = true;
    Rank rank;
    RR(Rank priority, ...) : rank(priority) {}
    Rank priority() { return this->rank; }
};

template <typename T>
struct RateMonotonic {
    using Rank                  = Microsecond;
    static constexpr bool Timed = true;
    Rank rank;
    RateMonotonic(Rank priority) : rank(priority) {}
    RateMonotonic(Microsecond, Microsecond p, Microsecond) : rank(p) {}
    Rank priority() { return this->rank; }
};

template <typename T>
struct Scheduler : POFO<T *> {
    using Queue = POFO<T *>;
    using Queue::empty;
    using Queue::insert;
    using Queue::next;
    using Queue::remove;
    using Queue::size;

    T *chose() {
        typename Queue::Element *e = next();
        if (e) return e->value;
        return nullptr;
    }
};
