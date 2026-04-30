#ifndef __DEPOS_KERNEL_SCHEDULER_FIXED_CORE_HEADER__
#define __DEPOS_KERNEL_SCHEDULER_FIXED_CORE_HEADER__

#include <kernel/scheduler/Policy.hpp>
#include <shared/synchronization/Spin.hpp>
#include <types.hpp>
#include <utils/collections/FIFO.hpp>

namespace DEPOS {

class FixedCore {
  public:
    enum : size_t { ANY = ~0U };
    enum : size_t { IDLE = 0, NORMAL, HIGHER };
    static constexpr size_t NumberOfCores  = Traits<CPU>::Active;
    static constexpr size_t NumberOfQueues = NumberOfCores * HIGHER;

    FixedCore(size_t rank = NORMAL, size_t cpu = ANY, ...)
        : m_index(build(rank, cpu)) {}

    template <typename T> struct Collection {
        void insert(const FixedCore &c, T *t) {
            size_t i = c.index();
            ERROR(i >= NumberOfQueues);
            m_locks[i].acquire();
            m_queues[i].insert(t);
            m_locks[i].release();
        }

        T *remove(size_t rank) {
            size_t i = encode(rank, CPU::id());
            ERROR(i >= NumberOfQueues);
            m_locks[i].acquire();
            T *t = m_queues[i].remove();
            m_locks[i].release();
            return t;
        }

      private:
        collections::FIFO<T> m_queues[NumberOfQueues];
        Spin m_locks[NumberOfQueues];
    };

    operator size_t() const { return rank(); }

  private:
    size_t index() const { return m_index; }
    size_t cpu() const { return index() / HIGHER; }
    size_t rank() const { return index() % HIGHER; }
    static size_t encode(size_t rank, size_t cpu) { return cpu * HIGHER + rank; }
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
