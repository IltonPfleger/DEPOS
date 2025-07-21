#pragma once
#include <Scheduler/Lists.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <Types.hpp>

// template <typename T>
// struct RR {
//     using Queue                 = FIFO<T *>;
//     using Rank                  = decltype(Queue::Element::rank);
//     static constexpr bool Timed = false;
//     static Rank rank(const T &value) { return value.rank; }
// };

template <typename T>
struct RateMonotonic {
    using Rank = Microsecond;
    Rank rank;
    static constexpr bool Timed = true;
    RateMonotonic(Rank priority) : rank(priority) {}
    RateMonotonic(Microsecond, Microsecond p, Microsecond) : rank(p) {}
    Rank priority() { return this->rank; }
};

template <typename T>
struct Scheduler : POFO<T *> {
    using Queue = POFO<T *>;
    using Queue::empty;
    using Queue::next;
    using Queue::remove;

    void insert(Queue::Element *value) { Queue::insert(value); }
    T *chose() { return next()->value; }
};
