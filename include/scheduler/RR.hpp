#ifndef __DEPOS_SCHEDULER_RR_HEADER__
#define __DEPOS_SCHEDULER_RR_HEADER__

#include <Spin.hpp>
#include <types.hpp>
#include <utils/Lists.hpp>

namespace DEPOS {

class RR {

  public:
    enum : uint8_t { IDLE = 0, NORMAL = 1, MAX = 2 };

    RR(uint8_t rank)
        : _rank(rank) {}

    operator size_t() const { return _rank; }

    template <typename T> struct Collection {
      public:
        void insert(const RR &r, T *t) {
            m_lock[r].acquire();
            m_queues[r].insert(t);
            m_lock[r].release();
        }

        T *remove(const RR &r) {
            m_lock[r].acquire();
            auto t = m_queues[r].remove();
            m_lock[r].release();
            return t;
        }

      private:
        FIFO<T> m_queues[MAX];
        Spin m_lock[MAX];
    };

  private:
    uint8_t _rank;
};

} // namespace DEPOS

#endif
