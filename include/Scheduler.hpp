#pragma once
#include <IO/Debug.hpp>
#include <Traits.hpp>
#include <utils/Lists.hpp>

template <typename U>
struct Head {};

template <>
struct Head<Thread> {
    static constexpr unsigned long N = Traits::Machine::CPUS;
    static auto id() { return Machine::CPU::core(); }
};

class Policy {
   public:
    using Rank = unsigned long;
    Policy(Rank r, ...) : rank_(r) {}
    Rank operator()() const { return rank_; }

   private:
    Rank rank_;
};

class RR : public Policy {
   public:
    static constexpr bool Timed = true;
    template <typename T>
    using Queue = POFO<T>;
    enum : Rank { NORMAL, IDLE = ~0ULL };
    RR(Rank r = NORMAL, ...) : Policy(r) {}
};

template <typename T>
class Scheduler : private Traits::Scheduler<T>::Criterion::template Queue<T *>, public Head<T> {
   public:
    using Criterion = typename Traits::Scheduler<T>::Criterion;
    using Queue     = typename Criterion::template Queue<T *>;
    using Node      = typename Queue::Node;
    using Queue::empty;
    using Queue::insert;

    T *pop() {
        auto e = this->next();
        ERROR(!e);
        heads_[this->id()] = e;
        return e->value;
    }

    T *current() {
        ERROR(!heads_[this->id()]);
        return heads_[this->id()]->value;
    }

   private:
    Node *heads_[Head<T>::N];
};
