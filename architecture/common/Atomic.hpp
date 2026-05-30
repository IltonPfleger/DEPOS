#pragma once

namespace DEPOS {

namespace ArchitectureCommon {

class Atomic {
  public:
    static auto tsl(auto &v) { return __atomic_test_and_set(&v, __ATOMIC_SEQ_CST); }
    static auto fdec(auto &v, unsigned int x = 1) { return __atomic_fetch_sub(&v, x, __ATOMIC_SEQ_CST); }
    static auto finc(auto &v, unsigned int x = 1) { return __atomic_fetch_add(&v, x, __ATOMIC_SEQ_CST); }
    static auto store(auto &v, auto x) { return __atomic_store_n(&v, x, __ATOMIC_RELEASE); }
    static auto load(auto &v) { return __atomic_load_n(&v, __ATOMIC_CONSUME); }

    template <typename T> static bool cas(T &value, T expected, T desired) {
        return __atomic_compare_exchange_n(&value, &expected, desired, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE);
    }
};

} // namespace ArchitectureCommon

} // namespace DEPOS
