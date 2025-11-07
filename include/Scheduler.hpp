#pragma once
#include <IO/Debug.hpp>
#include <Traits.hpp>
#include <utils/Lists.hpp>

template <typename U>
struct Head {};

template <>
struct Head<Thread> {
    static constexpr unsigned long N = Traits::Machine::CPUS;
    static auto get() { return Machine::CPU::core(); }
};

class RR {
    using Rank = unsigned long;

   public:
    template <typename T>
    using Queue                 = POFO<T>;
    static constexpr bool Timed = true;

   public:
    enum : Rank { NORMAL, IDLE = ~0ULL };
    RR(Rank r = NORMAL, ...) : rank_(r) {}
    Rank operator()() const { return rank_; }

   private:
    Rank rank_;
};

template <typename T>
class Scheduler : private Traits::Scheduler<T>::Criterion::template Queue<T *> {
   public:
    using Criterion = Traits::Scheduler<T>::Criterion;
    using Queue     = typename Criterion::template Queue<T *>;
    using Node      = Queue::Node;
    using Queue::empty;
    using Queue::insert;

    T *pop() {
        auto e = this->next();
        ERROR(!e);
        return e->value;
    }

    T *current() { return heads_[Head<T>::get()]; }

   private:
    T *heads_[Head<Thread>::N];
};
