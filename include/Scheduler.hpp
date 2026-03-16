#pragma once
#include <Spin.hpp>
#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <utils/Debug.hpp>
#include <utils/Lists.hpp>

namespace DEPOS {

class Policy {
  public:
    using Rank = int;
    Policy(Rank r, ...)
        : m_rank(r) {}
    operator Rank() const { return m_rank; }

  private:
    Rank m_rank;
};

class RR : public Policy {
  public:
    using Policy::Policy;
    template <typename T> using Queue = FIFO<T>;
    static constexpr int Width        = 1;
    static constexpr int Depth        = 2;
    static constexpr bool Preemptive  = true;
    enum { IDLE = 0, NORMAL = 1 };
    static int affinity(RR &&) { return 0; }
    static int affinity() { return 0; }
};

class FixedCPU : public Policy {
  public:
    enum { IDLE = 0, NORMAL = 1, ANY = ~0 };

    static constexpr int Width       = (Traits<CPU>::Active > 0) ? Traits<CPU>::Active : 1;
    static constexpr int Depth       = 2;
    static constexpr bool Preemptive = true;

    FixedCPU(Rank r, int width = ANY, ...)
        : Policy(r) {
        if ((int)*this == IDLE) {
            m_width = CPU::Atomic::finc(s_idles) % Width;
        } else if (width == ANY) {
            m_width = CPU::Atomic::finc(s_counter) % Width;
        } else {
            m_width = width % Width;
        }
    }

    template <typename T> using Queue = FIFO<T>;

    static int affinity(const FixedCPU &self) { return self.m_width; }
    static int affinity() { return CPU::id(); }

  private:
    static inline int s_idles   = 0;
    static inline int s_counter = 0;
    int m_width                 = 0;
};

template <typename T> class Scheduler {

  public:
    using Criterion = typename Traits<T>::Criterion;
    using Link      = Node<Thread *, Criterion>;
    using Queue     = typename Criterion::template Queue<Link>;

    Scheduler() = default;

    Link *remove(Criterion::Rank threshold = Criterion::IDLE) {
        int i        = Criterion::Depth - 1;
        int affinity = Criterion::affinity();

        while (i >= threshold) {
            m_lock[affinity].acquire();
            if (Link *next = m_levels[affinity][i].remove()) {
                m_lock[affinity].release();
                m_heads[head()] = next->value();
                return next;
            }
            m_lock[affinity].release();
            i--;
        }

        return nullptr;
    }

    void insert(Link *node) {
        ERROR(!node);
        m_lock[Criterion::affinity(node->criterion())].acquire();
        m_levels[Criterion::affinity(node->criterion())][node->criterion()].insert(node);
        m_lock[Criterion::affinity(node->criterion())].release();
    }

    auto head() { return CPU::id(); }

    auto *current() { return m_heads[head()]; }

  private:
    static constexpr unsigned int CPUS = Traits<CPU>::Active;

  private:
    T *m_heads[CPUS] = {nullptr};
    Queue m_levels[Criterion::Width][Criterion::Depth];
    Spin m_lock[Criterion::Width];
};

} // namespace DEPOS
