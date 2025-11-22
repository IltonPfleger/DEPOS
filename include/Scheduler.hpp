#pragma once
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utils/Debug.hpp>
#include <utils/Lists.hpp>

template <typename T> struct Head {};

template <> struct Head<Thread> {
    static constexpr unsigned long N = Traits<Machine>::CPUS;
    static auto id() { return CPU::id(); }
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
    static constexpr bool Preemptive = true;
    static constexpr int Levels = 2;
    template <typename T> using Queue = POFO<T>;
    enum : Rank { NORMAL, IDLE };
    RR(Rank r = NORMAL, ...) : Policy(r) {}
};

template <typename T> class Scheduler : public Head<T> {

  public:
    using Criterion = typename Traits<Scheduler<T>>::Criterion;
    using Queue = typename Criterion::template Queue<T *>;
    using Node = typename Queue::Node;

    static_assert(Traits<Scheduler<T>>::Preemptive == Criterion::Preemptive);

    bool empty() {
        for (auto &l : m_levels) {
            if (!l.empty())
                return false;
        }
        return true;
    }

    void insert(Node *n) { m_levels[0].insert(n); }

    T *pop() {
        auto e = m_levels[0].remove();
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
    Queue m_levels[Criterion::Levels];
};
