#pragma once
#include <IO/Debug.hpp>
#include <Scheduler/Lists.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <Types.hpp>

template <typename T>
struct RR {
    static constexpr bool Timed = true;
    using Rank                  = uintmax_t;
    Rank rank;

    enum : Rank { IDLE, NORMAL };
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
struct Scheduler : POFO<T *> {
    using Queue = POFO<T *>;
    using Queue::empty;
    using Queue::insert;
    using Queue::next;
    using Queue::remove;

    T *chose() {
        auto e = next();
        ERROR(!e, "[Scheduler<T>::chose] Empty queue.");
        return e->value;
    }
};
