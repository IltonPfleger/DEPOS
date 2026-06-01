#ifndef __DEPOS_ATOMIC__
#define __DEPOS_ATOMIC__

#include <Meta.hpp>
#include <architecture/CPU.hpp>

namespace DEPOS {

template <typename T> class Atomic {
  public:
    Atomic(T value = 0)
        : value_(value) {}

    T finc()
        requires Meta::Integer<T>
    {
        return CPU::Atomic::finc(value_);
    }

    T fdec()
        requires Meta::Integer<T>
    {
        return CPU::Atomic::fdec(value_);
    }

    bool cas(T &expected, T desired) { return CPU::Atomic::cas(value_, expected, desired); }

    T load() const { return CPU::Atomic::load(value_); }

    void store(T value) { CPU::Atomic::store(value_, value); }

    T operator++(int) { return finc(); }
    T operator--(int) { return fdec(); }
    operator T() const { return load(); }

  private:
    T value_;
};

} // namespace DEPOS

#endif
