#pragma once

namespace DEPOS {

namespace ArchitectureCommon {

class Atomic {
  public:
    template <typename T> static bool tsl(T &v) { return __atomic_test_and_set(&v, __ATOMIC_SEQ_CST); }

    template <typename T> static T fdec(T &v, T x = 1) { return __atomic_fetch_sub(&v, x, __ATOMIC_SEQ_CST); }

    template <typename T> static T finc(T &v, T x = 1) { return __atomic_fetch_add(&v, x, __ATOMIC_SEQ_CST); }

    template <typename T, typename U> static void store(T &v, U x) { __atomic_store_n(&v, x, __ATOMIC_RELEASE); }

    template <typename T> static T load(T &v) { return __atomic_load_n(&v, __ATOMIC_CONSUME); }

    template <typename T> static bool cas(T &v, T &expected, T desired) {
        return __atomic_compare_exchange_n(&v, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    }

    template <typename T, typename U> static T exchange(T &v, U x) {
        return __atomic_exchange_n(&v, x, __ATOMIC_SEQ_CST);
    }
};

} // namespace ArchitectureCommon

} // namespace DEPOS
