#ifndef __SCHEDULER_RR_HEADER__
#define __SCHEDULER_RR_HEADER__

#include <kernel/scheduler/Policy.hpp>
#include <shared/synchronization/Spin.hpp>
#include <types.hpp>
#include <utils/collections/FIFO.hpp>

namespace DEPOS {

class RR : public Policy<uint8_t> {
    using Base = Policy<uint8_t>;

  public:
    RR(uint8_t rank)
        : Base(rank) {}

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

} // namespace DEPOS

#endif
