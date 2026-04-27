#ifndef __SCHEDULER_FIXED_CORE_HEADER__
#define __SCHEDULER_FIXED_CORE_HEADER__

#include <kernel/scheduler/Policy.hpp>
#include <shared/synchronization/Spin.hpp>
#include <types.hpp>
#include <utils/collections/FIFO.hpp>
#include <utils/collections/MPSC.hpp>

// TODO: Explain Why It's Works Without ABA Problem in MPSC

namespace DEPOS {

class FixedCore {
  public:
    static constexpr size_t NumberOfCores  = Traits<CPU>::Active;
    static constexpr size_t Levels         = 2;
    static constexpr size_t NumberOfQueues = NumberOfCores * Levels;

    enum : size_t { ANY = ~0U };
    enum : size_t { IDLE = 0, NORMAL = 1 };

    FixedCore(size_t rank = NORMAL, size_t cpu = ANY, ...)
        : m_index(build(rank, cpu)) {}

    template <typename T> struct Collection {
        void insert(const FixedCore &c, T *t) { m_queues[c.index()].insert(t); }

        T *remove(const FixedCore &c) {
            auto i = encode(c.rank(), CPU::id());
            return m_queues[i].remove();
        }

      private:
        collections::MPSC<T> m_queues[NumberOfQueues];
    };

    operator size_t() const { return rank(); }

  private:
    size_t index() const { return m_index; }
    size_t cpu() const { return index() / Levels; }
    size_t rank() const { return index() % Levels; }
    static size_t encode(size_t rank, size_t cpu) { return cpu * Levels + rank; }
    static size_t build(size_t rank, size_t cpu) {
        if (rank == IDLE) {
            cpu = CPU::Atomic::finc(s_idles);
        } else if (cpu == ANY) {
            cpu = CPU::Atomic::finc(s_counter);
        }
        return encode(rank, cpu % NumberOfCores);
    }

  private:
    static inline size_t s_idles   = 0;
    static inline size_t s_counter = 0;
    size_t m_index;
};

} // namespace DEPOS

#endif
