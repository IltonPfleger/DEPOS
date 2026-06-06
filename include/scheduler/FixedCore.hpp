#ifndef __DEPOS_SCHEDULER_FIXED_CORE__
#define __DEPOS_SCHEDULER_FIXED_CORE__

#include <Spin.hpp>
#include <types.hpp>
#include <utility/Console.hpp>
#include <utility/collections/FIFO.hpp>

namespace DEPOS {

class FixedCore {
  public:
    enum : size_t { ANY = ~0U };
    enum : size_t { IDLE = 0, NORMAL, SYSTEM = NORMAL, HIGHER };
    static constexpr size_t NumberOfCores  = Traits<CPU>::Active;
    static constexpr size_t NumberOfQueues = NumberOfCores * HIGHER;

    FixedCore(size_t rank = NORMAL, size_t cpu = ANY, ...)
        : index_(build(rank, cpu)) {}

    FixedCore(const FixedCore &other)
        : index_(other.index_) {}

    FixedCore &operator=(const FixedCore &other) {
        if (this != &other) index_ = other.index_;
        return *this;
    }

    template <typename T> struct Collection {
        void insert(const FixedCore &c, T *t) {
            size_t i = c.index();
            locks_[i].acquire();
            queues_[i].insert(t);
            locks_[i].release();
        }

        T *remove(size_t rank) {
            size_t i = encode(rank, CPU::id());
            locks_[i].acquire();
            T *t = queues_[i].remove();
            locks_[i].release();
            return t;
        }

      private:
        collections::FIFO<T> queues_[NumberOfQueues];
        Spin locks_[NumberOfQueues];
    };

    operator size_t() const { return rank(); }

  private:
    size_t index() const { return index_; }
    size_t cpu() const { return index() / HIGHER; }
    size_t rank() const { return index() % HIGHER; }
    static size_t encode(size_t rank, size_t cpu) { return cpu * HIGHER + rank; }
    static size_t build(size_t rank, size_t cpu) {
        if (rank == IDLE && cpu == ANY) {
            cpu = CPU::Atomic::finc(idles_);
        } else if (rank == SYSTEM && cpu == ANY) {
            cpu = Traits<CPU>::BSP;
        } else if (cpu == ANY) {
            cpu = CPU::id();
        }
        return encode(rank, cpu % NumberOfCores);
    }

  private:
    static constinit inline size_t idles_   = 0;
    static constinit inline size_t counter_ = 0;
    size_t index_;
};

} // namespace DEPOS

#endif
