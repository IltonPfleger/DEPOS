#pragma once
#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <shared/console/Debug.hpp>
#include <utils/collections/FIFO.hpp>
#include <utils/collections/LIFO.hpp>
#include <utils/collections/Node.hpp>

#include <shared/synchronization/Spin.hpp>

namespace DEPOS {

template <typename T> class Policy {
  public:
    Policy(T r, ...)
        : m_rank(r) {}
    operator T() const { return m_rank; }

  private:
    T m_rank;
};

class RR : public Policy<uint8_t> {
    using Base = Policy<uint8_t>;

  public:
    using Base::Base;

    enum : uint8_t { IDLE = 0, NORMAL = 1, MAX = 2 };

    template <typename T> struct Collection {
      public:
        void insert(const RR &r, T *t) {
            ERROR(r >= MAX);
            m_lock[r].acquire();
            m_queues[r].insert(t);
            m_lock[r].release();
        }

        T *remove(const RR &r) {
            ERROR(r >= MAX);
            m_lock[r].acquire();
            auto t = m_queues[r].remove();
            m_lock[r].release();
            return t;
        }

      private:
        collections::FIFO<T> m_queues[MAX];
        Spin m_lock[MAX];
    };
};

class FixedCPU : public Policy<unsigned int> {
    using Base = Policy<unsigned int>;

  public:
    enum : unsigned int { CPUS = Traits<CPU>::Active, ANY = ~0U };
    enum : unsigned int { IDLE = 0, NORMAL = 1, MAX = 2 };

    FixedCPU(unsigned int r, unsigned int cpu = ANY, ...)
        : Policy(r) {
        if (*this == IDLE) {
            m_cpu = CPU::Atomic::finc(s_idles) % CPUS;
        } else if (cpu == ANY) {
            m_cpu = CPU::Atomic::finc(s_counter) % CPUS;
        } else {
            m_cpu = cpu % CPUS;
        }
    }

    template <typename T> struct Collection {
      public:
        void insert(const FixedCPU &c, T *t) {
            unsigned int cpu = c.m_cpu;
            unsigned int r   = c;
            ERROR(r >= MAX);
            ERROR(cpu >= CPUS);

            m_lock[cpu][r].acquire();
            m_queues[cpu][r].insert(t);
            m_lock[cpu][r].release();
        }

        T *remove(const FixedCPU &c) {
            unsigned int cpu = CPU::id();
            unsigned int r   = c;
            ERROR(r >= MAX);
            ERROR(cpu >= CPUS);

            m_lock[cpu][r].acquire();
            T *t = m_queues[cpu][r].remove();
            m_lock[cpu][r].release();
            return t;
        }

      private:
        collections::FIFO<T> m_queues[CPUS][MAX];
        Spin m_lock[CPUS][MAX];
    };

  private:
    static inline unsigned int s_idles   = 0;
    static inline unsigned int s_counter = 0;
    unsigned int m_cpu                   = 0;
};

template <typename T> class Scheduler {

  public:
    using Criterion = typename Traits<T>::Criterion;
    using Link      = collections::Node<T *, Criterion>;

    Scheduler() = default;

    Link *remove(Criterion threshold = Criterion::IDLE) {
        unsigned int i = Criterion::MAX - 1;

        while (i >= threshold) {
            if (Link *next = m_collection.remove(i)) {
                m_heads[head()] = next->value();
                return next;
            }
            i--;
        }

        return nullptr;
    }

    void insert(Link *node) {
        ERROR(!node);
        m_collection.insert(node->criterion(), node);
    }

    auto head() { return CPU::id(); }

    auto *current() { return m_heads[head()]; }

  private:
    static constexpr unsigned int CPUS = Traits<CPU>::Active;

  private:
    Meta::Array<CPUS, T *> m_heads;
    typename Criterion::template Collection<Link> m_collection;
};

} // namespace DEPOS
