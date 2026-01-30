#pragma once
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utils/Debug.hpp>
#include <utils/Lists.hpp>

template <typename T> struct Head {};

template <> struct Head<Thread> {
    static constexpr unsigned long N = Traits<CPUS>::COUNT;
    static auto id() { return CPU::id(); }
};

class Policy {
  public:
    using Rank = unsigned int;
    Policy(Rank r, ...) : m_rank(r) {}
    operator Rank() const { return m_rank; }

  private:
    Rank m_rank;
};

class RR : public Policy {
  public:
    static constexpr bool Preemptive = true;
    template <typename T> using Queue = LIFO<T>;
    enum : Rank {
        NORMAL,
        IDLE,
        Levels,
    };
    RR(Rank r = NORMAL, ...) : Policy(r) {}
};

template <typename T> class Scheduler : private Head<T> {

  public:
    using Criterion = typename Traits<Scheduler<T>>::Criterion;
    using Entry = Node<Thread *, Criterion>;
    using Queue = typename Criterion::template Queue<Entry>;

    bool empty() const {
        for (unsigned int i = 0; i < Criterion::Levels; i++) {
            if (!m_levels[i].empty()) return false;
        }
        return true;
    }

    T *pop() {
        T *next = nullptr;
        for (unsigned int i = 0; i < Criterion::Levels; i++) {
            if (!m_levels[i].empty()) {
                Entry *node = m_levels[i].remove();
                next = node->value;
                break;
            }
        }
        ERROR(!next);
        m_heads[this->id()] = next;
        return next;
    }

    void insert(Entry *node) {
        ERROR(node->priority >= Criterion::Levels);
        m_levels[node->priority].insert(node);
    }

    T *current() const {
        ERROR(!m_heads[this->id()]);
        return m_heads[this->id()];
    }

  private:
    T *m_heads[Head<T>::N];
    Queue m_levels[Criterion::Levels];
};
