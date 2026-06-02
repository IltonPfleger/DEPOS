#ifndef __DEPOS_ATOMIC_BOUNDED_SIMPLE_LIST__
#define __DEPOS_ATOMIC_BOUNDED_SIMPLE_LIST__

#include <utility/Atomic.hpp>

namespace DEPOS {

template <typename T, size_t Capacity, size_t Internal = 0> class AtomicBoundedSimpleList {
  public:
    constexpr AtomicBoundedSimpleList()
        : head_(0),
          tail_(0),
          size_(0) {
        for (size_t i = 0; i < Internal; i++) {
            insert(&pool_[i]);
        }
    }

    AtomicBoundedSimpleList(const AtomicBoundedSimpleList &)            = delete;
    AtomicBoundedSimpleList &operator=(const AtomicBoundedSimpleList &) = delete;

    bool remove(T **pointer) {
        size_t old;
        while (true) {
            old = size_;
            if (old == 0) return false;
            if (size_.cas(old, old - 1)) break;
        };
        old      = head_.finc() % Capacity;
        *pointer = list_[old];
        return true;
    }

    bool insert(T *pointer) {
        size_t old;
        while (true) {
            old = size_;
            if (old >= Capacity) return false;
            if (size_.cas(old, old + 1)) break;
        };

        old        = tail_.finc() % Capacity;
        list_[old] = pointer;
        return true;
    }

  private:
    Atomic<size_t> head_;
    Atomic<size_t> tail_;
    Atomic<size_t> size_;
    T *list_[Capacity];
    T pool_[Internal];
};

} // namespace DEPOS

#endif
