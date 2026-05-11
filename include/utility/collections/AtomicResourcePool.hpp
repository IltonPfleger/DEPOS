#ifndef __DEPOS_ATOMIC_RESOURCE_POOL__
#define __DEPOS_ATOMIC_RESOURCE_POOL__

#include <Meta.hpp>

namespace DEPOS {

namespace collections {

template <typename T, size_t Capacity> class AtomicResourcePool {

  public:
    AtomicResourcePool() {
        for (size_t i = 0; i < Capacity; i++)
            allocated_[i] = false;
    }

    template <typename... Args> void construct(size_t i, Args &&...args) {
        new (&cells_[i]) T(static_cast<Args &&>(args)...);
    }

    void insert(T &cell) {
        size_t i      = &cell - &cells_[0];
        allocated_[i] = false;
    }

    T &remove() {
        for (size_t i = 0;; i = (i + 1) % Capacity) {
            if (!CPU::Atomic::tsl(allocated_[i])) return cells_[i];
        }
    }

  private:
    Meta::Array<Capacity, T> cells_;
    Meta::Array<Capacity, bool> allocated_;
};

} // namespace collections

} // namespace DEPOS

#endif
