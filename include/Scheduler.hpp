#pragma once
#include <Spin.hpp>
#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <utils/Debug.hpp>
#include <utils/Guard.hpp>
#include <utils/Lists.hpp>

namespace DEPOS {

class Policy {
  public:
    using Rank = int;
    Policy(Rank r, ...) : m_rank(r) {}
    operator Rank() const { return m_rank; }

  private:
    Rank m_rank;
};

class RR : public Policy {
  public:
    static constexpr bool Preemptive = true;
    template <typename T> using Queue = FIFO<T>;
    enum : Rank { IDLE = 0, NORMAL = 1, Levels = 2 };
    RR(Rank r = NORMAL, ...) : Policy(r) {}
};

template <typename T> class Scheduler {

  public:
    using Criterion = typename Traits<Scheduler<T>>::Criterion;
    using Element = Node<Thread *, Criterion>;
    using Queue = typename Criterion::template Queue<Element>;

    Scheduler() = default;

    Element *remove(Criterion::Rank threshold = Criterion::IDLE) {
        Element *next = nullptr;
        int i = Criterion::Levels - 1;
        while (i >= threshold && !next) {
            m_spin.acquire();
            next = m_levels[i].remove();
            m_spin.release();
            i = i - 1;
        }
        if (next) m_heads[CPU::id()] = next->value();
        return next;
    }

    void insert(Element *node) {
        ERROR(!node);
        ERROR(node->priority() >= Criterion::Levels);
        m_spin.acquire();
        m_levels[node->priority()].insert(node);
        m_spin.release();
    }

    T *current() { return m_heads[CPU::id()]; }

  private:
    T *m_heads[Traits<CPU>::Active];
    Queue m_levels[Criterion::Levels];
    Spin m_spin;
};

} // namespace DEPOS
