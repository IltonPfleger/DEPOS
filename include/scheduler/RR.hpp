#ifndef __DEPOS_SCHEDULER_RR__
#define __DEPOS_SCHEDULER_RR__

#include <Spin.hpp>
#include <types.hpp>
#include <utility/collections/FIFO.hpp>

namespace DEPOS {

class RR {

  public:
    enum : uint8_t { IDLE = 0, NORMAL = 1, MAX = 2 };

    RR(uint8_t rank)
        : rank_(rank) {}

    operator size_t() const { return rank_; }

    template <typename T> struct Collection {
      public:
        void insert(const RR &r, T *t) {
            lock_[r].acquire();
            queues_[r].insert(t);
            lock_[r].release();
        }

        T *remove(const RR &r) {
            lock_[r].acquire();
            auto t = queues_[r].remove();
            lock_[r].release();
            return t;
        }

      private:
        collections::FIFO<T> queues_[MAX];
        Spin lock_[MAX];
    };

  private:
    uint8_t rank_;
};

} // namespace DEPOS

#endif
