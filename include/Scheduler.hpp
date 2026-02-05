#pragma once
#include <Spin.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utils/Debug.hpp>
#include <utils/Guard.hpp>
#include <utils/Lists.hpp>

// template <typename T> struct Head {};
//
// template <> struct Head<Thread> {
//     static constexpr unsigned long N = Traits<CPUS>::COUNT;
//     static auto id() { return CPU::id(); }
// };

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
    enum : Rank {
        IDLE = 0,
        NORMAL = 1,
        Levels,
    };
    RR(Rank r = NORMAL, ...) : Policy(r) {}
};

template <typename T> class Scheduler {

  public:
    using Criterion = typename Traits<Scheduler<T>>::Criterion;
    using Element = Node<Thread *, Criterion>;
    using Queue = typename Criterion::template Queue<Element>;

    // bool empty() const {
    //     for (unsigned int i = 0; i < Criterion::Levels; i++) {
    //         if (!m_levels[i].empty()) return false;
    //     }
    //     return true;
    // }

    Element *remove(Criterion::Rank threshold = 0) {
        Guard<Spin, &Spin::acquire, &Spin::release> G(&m_spin);
        for (int i = Criterion::Levels - 1; i >= threshold; i--) {
            if (!m_levels[i].empty()) {
                Element *next = m_levels[i].remove();
                m_heads[CPU::id()] = next->value();
                return next;
            }
        }
        return nullptr;
    }

    void insert(Element *node) {
        Guard<Spin, &Spin::acquire, &Spin::release> G(&m_spin);
        ERROR(!node);
        ERROR(node->priority() >= Criterion::Levels);
        m_levels[node->priority()].insert(node);
    }

    T *current() {
        Guard<Spin, &Spin::acquire, &Spin::release> G(&m_spin);
        ERROR(!m_heads[CPU::id()]);
        return m_heads[CPU::id()];
    }

  private:
    T *m_heads[Traits<CPUS>::ACTIVE];
    Queue m_levels[Criterion::Levels];
    Spin m_spin;
};
