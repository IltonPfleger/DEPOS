#pragma once
#include <IO/Debug.hpp>
#include <Lists.hpp>
#include <Traits.hpp>
#include <Types.hpp>

template <typename T>
struct RR {
    static constexpr bool Timed = true;
    using Rank                  = uintmax_t;
    Rank rank;

    enum : Rank { NORMAL, IDLE = ~0ULL };
    RR(Rank priority = NORMAL, ...) : rank(priority) {}
    Rank priority() const { return rank; }
};

// template <typename T>
// struct RateMonotonic {
//     using Rank                  = Microsecond;
//     static constexpr bool Timed = true;
//     Rank rank;
//     RateMonotonic(Rank priority) : rank(priority) {}
//     RateMonotonic(Microsecond, Microsecond p, Microsecond) : rank(p) {}
//     Rank priority() const { return this->rank; }
// };

template <typename T>
struct Scheduler : private POFO<T *> {
    using Queue = POFO<T *>;
    using Queue::empty;
    using Queue::remove;

    void push(Queue::Node *n) { Queue::insert(n); }

    T *pop() {
        auto e = Queue::next();
        ERROR(!e, "[Scheduler<T>::pop] Empty.");
        return e->value;
    }
};
