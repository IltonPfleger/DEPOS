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

class RR : public Priority {
  public:
    static constexpr bool Preemptive = true;
    static constexpr int Levels = 2;
    template <typename T> using Queue = MLQ<LIFO<Element<T, RR>>, Levels>;
    enum { NORMAL, IDLE };
    RR(Priority::Type r = NORMAL, ...) : Priority(r) {}
};

template <typename T> class Scheduler : private Traits<Scheduler<T>>::Criterion::template Queue<T *>, public Head<T> {
  public:
    using Criterion = typename Traits<Scheduler<T>>::Criterion;
    using Queue = typename Criterion::template Queue<T *>;
    using Node = typename Queue::Node;

    using Queue::empty;
    using Queue::insert;

    T *remove() {
        Node *n = Queue::remove();
        ERROR(!n);
        m_heads[this->id()] = n->value;
        return n->value;
    }

    T *current() {
        ERROR(!m_heads[this->id()]);
        return m_heads[this->id()];
    }

  private:
    T *m_heads[Head<T>::N];
};
