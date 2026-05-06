#ifndef __DEPOS_KERNEL_SCHEDULER_FIXED_CORE_HEADER__
#define __DEPOS_KERNEL_SCHEDULER_FIXED_CORE_HEADER__

#include <Spin.hpp>
#include <types.hpp>
#include <utils/Lists.hpp>

namespace DEPOS {

class FixedCore {
  public:
    enum : size_t { ANY = ~0U };
    enum : size_t { IDLE = 0, NORMAL, HIGHER };
    static constexpr size_t NumberOfCores  = Traits<CPU>::Active;
    static constexpr size_t NumberOfQueues = NumberOfCores * HIGHER;

    FixedCore(size_t rank = NORMAL, size_t cpu = ANY, ...)
        : _index(build(rank, cpu)) {}

    template <typename T> struct Collection {
        void insert(const FixedCore &c, T *t) {
            size_t i = c.index();
            _locks[i].acquire();
            _queues[i].insert(t);
            _locks[i].release();
        }

        T *remove(size_t rank) {
            size_t i = encode(rank, CPU::id());
            _locks[i].acquire();
            T *t = _queues[i].remove();
            _locks[i].release();
            return t;
        }

      private:
        FIFO<T> _queues[NumberOfQueues];
        Spin _locks[NumberOfQueues];
    };

    operator size_t() const { return rank(); }

  private:
    size_t index() const { return _index; }
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
    size_t _index;
};

} // namespace DEPOS

#endif
